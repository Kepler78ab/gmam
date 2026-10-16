#include "TcpClientWorker.h"

TcpClientWorker::TcpClientWorker(QObject* parent)
    : QObject(parent)
    , m_sendQueue(100)
{
}

TcpClientWorker::~TcpClientWorker()
{
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
    }
}

void TcpClientWorker::setAckMatchers(TcpFrameMatcher positive,
                                     TcpFrameMatcher negative,
                                     TcpFrameMatcher heartbeatIgnore)
{
    m_ackPositive = std::move(positive);
    m_ackNegative = std::move(negative);
    m_heartbeatIgnore = std::move(heartbeatIgnore);
}

void TcpClientWorker::setHeartbeatProvider(TcpHeartbeatFrameProvider provider)
{
    m_heartbeatProvider = std::move(provider);
}

void TcpClientWorker::setCustomSplitter(TcpCustomFrameSplitter splitter)
{
    m_assembler.setCustomSplitter(std::move(splitter));
}

void TcpClientWorker::slotClearReceiveBuffer()
{
    m_assembler.clear();
}

void TcpClientWorker::slotApplyConfig(TcpClientConfig config)
{
    // ack 开启却无 A/N matcher：强制关闭，避免无判定的停等死锁
    if (config.ackEnabled && (!m_ackPositive || !m_ackNegative)) {
        emit sigError(QStringLiteral("ackEnabled=true 但未设置 A/N matcher，已关闭 ack"));
        config.ackEnabled = false;
    }

    // PesAscii → LengthPrefix 固定参数（拆帧实现只认 LengthPrefix/Custom/Raw）
    if (config.frameMode == TcpFrameMode::PesAscii) {
        config.frameMode = TcpFrameMode::LengthPrefix;
        config.lengthPrefixOffset = 0;
        config.lengthPrefixSize = 6;
        config.lengthPrefixEncoding = TcpLengthPrefixEncoding::DecimalAscii;
        config.hasFrameTerminator = true;
        config.frameTerminator = static_cast<char>(0x0A);
    }

    m_config = std::move(config);
    m_sendQueue.setMaxSize(m_config.sendQueueMaxSize);
    m_assembler.setConfig(m_config);

    if (m_reconnectTimer)
        m_reconnectTimer->setInterval(qMax(100, m_config.reconnectIntervalMs));
    if (m_connectTimer)
        m_connectTimer->setInterval(qMax(100, m_config.connectTimeoutMs));

    ensureStrategyTimers();
    updateHeartbeatTimer();

    if (!m_config.ackEnabled)
        clearInFlight();
}

void TcpClientWorker::ensureSocket()
{
    if (m_socket)
        return;

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpClientWorker::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClientWorker::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClientWorker::onSocketReadyRead);
    connect(m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this,
            &TcpClientWorker::onSocketError);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(qMax(100, m_config.reconnectIntervalMs));
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpClientWorker::onReconnectTimeout);

    m_connectTimer = new QTimer(this);
    m_connectTimer->setSingleShot(true);
    m_connectTimer->setInterval(qMax(100, m_config.connectTimeoutMs));
    connect(m_connectTimer, &QTimer::timeout, this, &TcpClientWorker::onConnectTimeout);

    ensureStrategyTimers();
}

void TcpClientWorker::ensureStrategyTimers()
{
    if (!m_heartbeatTimer) {
        m_heartbeatTimer = new QTimer(this);
        connect(m_heartbeatTimer, &QTimer::timeout, this, &TcpClientWorker::onHeartbeatTimeout);
    }
    if (!m_ackTimer) {
        m_ackTimer = new QTimer(this);
        m_ackTimer->setSingleShot(true);
        connect(m_ackTimer, &QTimer::timeout, this, &TcpClientWorker::onAckTimeout);
    }
    if (!m_sendGapTimer) {
        m_sendGapTimer = new QTimer(this);
        m_sendGapTimer->setSingleShot(true);
        connect(m_sendGapTimer, &QTimer::timeout, this, &TcpClientWorker::onSendGapTimeout);
    }
}

void TcpClientWorker::scheduleSendGap()
{
    if (m_config.sendIntervalMs <= 0)
        return;
    ensureStrategyTimers();
    m_sendGapTimer->start(m_config.sendIntervalMs);
}

void TcpClientWorker::onSendGapTimeout()
{
    slotPumpSend();
}

void TcpClientWorker::updateHeartbeatTimer()
{
    ensureStrategyTimers();
    if (m_config.heartbeatEnabled && m_connected.load()) {
        m_heartbeatTimer->setInterval(qMax(100, m_config.heartbeatIntervalMs));
        if (!m_heartbeatTimer->isActive())
            m_heartbeatTimer->start();
    } else if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
    }
}

void TcpClientWorker::slotConnect()
{
    ensureSocket();
    m_userDisconnect = false;
    m_wantConnected = true;
    stopReconnectTimer();

    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return;
    if (m_socket->state() == QAbstractSocket::ConnectingState)
        return;

    abortConnecting();
    m_assembler.clear();
    m_socket->connectToHost(m_config.host, m_config.port);
    m_connectTimer->start();
}

void TcpClientWorker::slotDisconnect(bool userRequested)
{
    ensureSocket();
    m_userDisconnect = userRequested;
    if (userRequested)
        m_wantConnected = false;
    stopReconnectTimer();
    if (m_connectTimer)
        m_connectTimer->stop();
    if (m_heartbeatTimer)
        m_heartbeatTimer->stop();
    if (m_ackTimer)
        m_ackTimer->stop();
    if (m_sendGapTimer)
        m_sendGapTimer->stop();
    clearInFlight();

    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        m_connected = false;
        return;
    }
    m_socket->disconnectFromHost();
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->waitForDisconnected(1000);
}

void TcpClientWorker::slotNotifyQueueSize(int size)
{
    const int high = m_config.highWaterMark;
    const int low = m_config.lowWaterMark;
    if (high > 0 && size >= high) {
        if (!m_highWaterEmitted) {
            m_highWaterEmitted = true;
            emit sigQueueHighWater();
        }
    } else if (m_highWaterEmitted && low >= 0 && size <= low) {
        m_highWaterEmitted = false;
        emit sigQueueLowWater();
    }
}

void TcpClientWorker::slotPumpSend()
{
    if (!m_connected.load() || !m_socket)
        return;

    if (m_config.sendIntervalMs > 0 && m_lastSendValid) {
        if (m_lastSendTimer.elapsed() < m_config.sendIntervalMs) {
            scheduleSendGap();
            return;
        }
    }

    if (m_config.ackEnabled)
        pumpWithAck();
    else
        pumpPassThrough();
}

void TcpClientWorker::pumpPassThrough()
{
    QByteArray frame;
    while (m_sendQueue.tryDequeue(frame)) {
        writeOne(frame);
        if (m_config.sendIntervalMs > 0) {
            m_lastSendTimer.restart();
            m_lastSendValid = true;
            scheduleSendGap();
            break;
        }
    }
    slotNotifyQueueSize(m_sendQueue.size());
}

void TcpClientWorker::pumpWithAck()
{
    // 停等窗口=1：已有未确认帧则不再取下一帧
    if (m_inFlight)
        return;

    QByteArray frame;
    if (!m_sendQueue.tryPeek(frame))
        return;

    // peek 不出队：等 A/N 或超次后再 dequeue
    m_inFlight = true;
    m_inFlightFrame = frame;
    m_inFlightSendCount = 1;
    writeOne(m_inFlightFrame);
    m_lastSendTimer.restart();
    m_lastSendValid = true;
    ensureStrategyTimers();
    m_ackTimer->start(qMax(100, m_config.ackTimeoutMs));
}

void TcpClientWorker::slotShutdown()
{
    m_wantConnected = false;
    m_userDisconnect = true;
    stopReconnectTimer();
    if (m_connectTimer)
        m_connectTimer->stop();
    if (m_heartbeatTimer)
        m_heartbeatTimer->stop();
    if (m_ackTimer)
        m_ackTimer->stop();
    if (m_sendGapTimer)
        m_sendGapTimer->stop();
    clearInFlight();
    if (m_socket)
        m_socket->abort();
    m_connected = false;
    m_sendQueue.clear();
    m_assembler.clear();
}

void TcpClientWorker::onSocketConnected()
{
    if (m_connectTimer)
        m_connectTimer->stop();
    stopReconnectTimer();
    m_connected = true;
    m_assembler.clear();
    emit sigConnected();
    updateHeartbeatTimer();
    slotPumpSend();
}

void TcpClientWorker::onSocketDisconnected()
{
    if (m_connectTimer)
        m_connectTimer->stop();
    if (m_heartbeatTimer)
        m_heartbeatTimer->stop();
    if (m_ackTimer)
        m_ackTimer->stop();
    clearInFlight();

    const bool wasConnected = m_connected.exchange(false);
    if (wasConnected)
        emit sigDisconnected();

    if (m_wantConnected && m_config.autoReconnect && !m_userDisconnect)
        startReconnectTimer();
}

void TcpClientWorker::onSocketReadyRead()
{
    if (!m_socket)
        return;
    const QByteArray data = m_socket->readAll();
    if (data.isEmpty())
        return;

    emit sigBytesReceived(data);

    const QVector<QByteArray> frames = m_assembler.feed(data);
    const QString ferr = m_assembler.takeLastError();
    if (!ferr.isEmpty())
        emit sigError(ferr);

    for (const QByteArray& f : frames) {
        emit sigFrameReceived(f);
        dispatchFrame(f);
    }
}

void TcpClientWorker::dispatchFrame(const QByteArray& frame)
{
    // 对端心跳等：不驱动业务 wait-ack
    if (m_heartbeatIgnore && m_heartbeatIgnore(frame))
        return;

    if (!m_config.ackEnabled || !m_inFlight)
        return;

    if (m_ackPositive && m_ackPositive(frame)) {
        onAckPositive();
        return;
    }
    if (m_ackNegative && m_ackNegative(frame)) {
        onAckNegative(QStringLiteral("负应答"));
        return;
    }
}

void TcpClientWorker::onAckPositive()
{
    if (m_ackTimer)
        m_ackTimer->stop();
    QByteArray dummy;
    m_sendQueue.tryDequeue(dummy); // 出队当前队首
    clearInFlight();
    emit sigSendAccepted();
    slotNotifyQueueSize(m_sendQueue.size());
    slotPumpSend();
}

void TcpClientWorker::onAckNegative(const QString& reason)
{
    if (m_ackTimer)
        m_ackTimer->stop();
    QByteArray dummy;
    m_sendQueue.tryDequeue(dummy);
    clearInFlight();
    emit sigSendRejected(reason);
    slotNotifyQueueSize(m_sendQueue.size());
    slotPumpSend();
}

void TcpClientWorker::onAckTimeout()
{
    if (!m_inFlight)
        return;

    const int maxSend = qMax(1, m_config.ackMaxRetry);
    if (m_inFlightSendCount < maxSend) {
        ++m_inFlightSendCount;
        writeOne(m_inFlightFrame);
        m_ackTimer->start(qMax(100, m_config.ackTimeoutMs));
        return;
    }

    QByteArray dummy;
    m_sendQueue.tryDequeue(dummy);
    clearInFlight();
    emit sigSendRejected(QStringLiteral("应答超时，已达最大发送次数"));
    slotNotifyQueueSize(m_sendQueue.size());
    slotPumpSend();
}

void TcpClientWorker::onHeartbeatTimeout()
{
    if (!m_connected.load() || !m_config.heartbeatEnabled)
        return;

    QByteArray frame = m_config.heartbeatFrame;
    if (m_heartbeatProvider)
        frame = m_heartbeatProvider(); // provider 优先于固定帧
    if (frame.isEmpty())
        return;

    // 默认旁路 write：不入业务队列、不占 ack 窗口（heartbeatOccupyAckWindow 占位未另实现）
    Q_UNUSED(m_config.heartbeatOccupyAckWindow);
    writeOne(frame);
}

void TcpClientWorker::clearInFlight()
{
    m_inFlight = false;
    m_inFlightFrame.clear();
    m_inFlightSendCount = 0;
}

void TcpClientWorker::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    if (!m_socket)
        return;
    emit sigError(m_socket->errorString());

    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_connected = false;
        if (m_connectTimer)
            m_connectTimer->stop();
        if (m_heartbeatTimer)
            m_heartbeatTimer->stop();
        if (m_ackTimer)
            m_ackTimer->stop();
        clearInFlight();
        if (m_wantConnected && m_config.autoReconnect && !m_userDisconnect)
            startReconnectTimer();
    }
}

void TcpClientWorker::onReconnectTimeout()
{
    if (!m_wantConnected || m_userDisconnect)
        return;
    if (!m_socket)
        return;
    if (m_socket->state() == QAbstractSocket::ConnectedState
        || m_socket->state() == QAbstractSocket::ConnectingState)
        return;
    m_socket->connectToHost(m_config.host, m_config.port);
    if (m_connectTimer)
        m_connectTimer->start();
}

void TcpClientWorker::onConnectTimeout()
{
    if (!m_socket)
        return;
    if (m_socket->state() == QAbstractSocket::ConnectedState)
        return;
    emit sigError(QStringLiteral("连接超时"));
    m_socket->abort();
    m_connected = false;
    if (m_wantConnected && m_config.autoReconnect && !m_userDisconnect)
        startReconnectTimer();
}

void TcpClientWorker::startReconnectTimer()
{
    ensureSocket();
    if (!m_reconnectTimer->isActive())
        m_reconnectTimer->start();
}

void TcpClientWorker::stopReconnectTimer()
{
    if (m_reconnectTimer && m_reconnectTimer->isActive())
        m_reconnectTimer->stop();
}

void TcpClientWorker::abortConnecting()
{
    if (!m_socket)
        return;
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->abort();
}

void TcpClientWorker::writeOne(const QByteArray& frame)
{
    if (!m_socket || frame.isEmpty())
        return;
    m_socket->write(frame);
    m_socket->flush();
}
