#include "TcpClient.h"
#include "TcpClientWorker.h"

TcpClient::TcpClient(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<TcpClientConfig>("TcpClientConfig");
    m_config = TcpClientConfig::defaults();
}

TcpClient::~TcpClient()
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "slotShutdown", Qt::BlockingQueuedConnection);
    }
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(3000);
    }
}

void TcpClient::ensureWorkerStarted()
{
    if (m_started)
        return;

    m_worker = new TcpClientWorker;
    m_worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &TcpClientWorker::sigConnected, this, &TcpClient::connected);
    connect(m_worker, &TcpClientWorker::sigDisconnected, this, &TcpClient::disconnected);
    connect(m_worker, &TcpClientWorker::sigError, this, &TcpClient::errorOccurred);
    connect(m_worker, &TcpClientWorker::sigBytesReceived, this, &TcpClient::bytesReceived);
    connect(m_worker, &TcpClientWorker::sigFrameReceived, this, &TcpClient::frameReceived);
    connect(m_worker, &TcpClientWorker::sigQueueRejected, this, &TcpClient::queueRejected);
    connect(m_worker, &TcpClientWorker::sigQueueHighWater, this, &TcpClient::queueHighWater);
    connect(m_worker, &TcpClientWorker::sigQueueLowWater, this, &TcpClient::queueLowWater);
    connect(m_worker, &TcpClientWorker::sigSendAccepted, this, &TcpClient::sendAccepted);
    connect(m_worker, &TcpClientWorker::sigSendRejected, this, &TcpClient::sendRejected);

    m_thread.start();
    m_started = true;

    pushMatchersToWorker();
    pushProviderToWorker();
    pushSplitterToWorker();
    QMetaObject::invokeMethod(m_worker, "slotApplyConfig", Qt::QueuedConnection,
                              Q_ARG(TcpClientConfig, m_config));
}

void TcpClient::pushMatchersToWorker()
{
    if (!m_worker)
        return;
    QMetaObject::invokeMethod(
        m_worker,
        [w = m_worker, a = m_ackPositive, b = m_ackNegative, c = m_heartbeatIgnore]() {
            w->setAckMatchers(a, b, c);
        },
        Qt::BlockingQueuedConnection);
}

void TcpClient::pushProviderToWorker()
{
    if (!m_worker)
        return;
    QMetaObject::invokeMethod(
        m_worker,
        [w = m_worker, p = m_heartbeatProvider]() { w->setHeartbeatProvider(p); },
        Qt::BlockingQueuedConnection);
}

void TcpClient::pushSplitterToWorker()
{
    if (!m_worker)
        return;
    QMetaObject::invokeMethod(
        m_worker,
        [w = m_worker, s = m_customSplitter]() { w->setCustomSplitter(s); },
        Qt::BlockingQueuedConnection);
}

void TcpClient::setConfig(const TcpClientConfig& config)
{
    m_config = config;
    if (!m_started)
        return;
    QMetaObject::invokeMethod(m_worker, "slotApplyConfig", Qt::QueuedConnection,
                              Q_ARG(TcpClientConfig, m_config));
}

TcpClientConfig TcpClient::config() const
{
    return m_config;
}

void TcpClient::connectToHost()
{
    ensureWorkerStarted();
    pushMatchersToWorker();
    pushProviderToWorker();
    pushSplitterToWorker();
    QMetaObject::invokeMethod(m_worker, "slotApplyConfig", Qt::QueuedConnection,
                              Q_ARG(TcpClientConfig, m_config));
    QMetaObject::invokeMethod(m_worker, "slotConnect", Qt::QueuedConnection);
}

void TcpClient::disconnectFromHost()
{
    if (!m_started)
        return;
    QMetaObject::invokeMethod(m_worker, "slotDisconnect", Qt::QueuedConnection,
                              Q_ARG(bool, true));
}

bool TcpClient::isConnected() const
{
    if (!m_worker)
        return false;
    return m_worker->isConnectedFlag();
}

bool TcpClient::enqueue(const QByteArray& frame)
{
    if (frame.isEmpty())
        return false;

    ensureWorkerStarted();

    if (!m_config.sendQueueEnabled) {
        if (!isConnected())
            return false;
    }

    bool ok = false;
    if (m_config.sendQueueOverflowPolicy == TcpQueueOverflowPolicy::DropOldest)
        ok = m_worker->sendQueue()->tryEnqueueDropOldest(frame);
    else
        ok = m_worker->sendQueue()->tryEnqueue(frame);

    if (!ok) {
        emit queueRejected();
        QMetaObject::invokeMethod(m_worker, "slotPumpSend", Qt::QueuedConnection);
        return false;
    }

    const int sz = m_worker->sendQueue()->size();
    QMetaObject::invokeMethod(m_worker, "slotNotifyQueueSize", Qt::QueuedConnection,
                              Q_ARG(int, sz));
    QMetaObject::invokeMethod(m_worker, "slotPumpSend", Qt::QueuedConnection);
    return true;
}

void TcpClient::setHeartbeatFrame(const QByteArray& frame)
{
    m_config.heartbeatFrame = frame;
    if (m_started) {
        QMetaObject::invokeMethod(m_worker, "slotApplyConfig", Qt::QueuedConnection,
                                  Q_ARG(TcpClientConfig, m_config));
    }
}

void TcpClient::setHeartbeatFrameProvider(TcpHeartbeatFrameProvider provider)
{
    m_heartbeatProvider = std::move(provider);
    if (m_started)
        pushProviderToWorker();
}

void TcpClient::setAckMatchers(TcpFrameMatcher positive,
                               TcpFrameMatcher negative,
                               TcpFrameMatcher heartbeatIgnore)
{
    m_ackPositive = std::move(positive);
    m_ackNegative = std::move(negative);
    m_heartbeatIgnore = std::move(heartbeatIgnore);
    if (m_started)
        pushMatchersToWorker();
}

void TcpClient::setCustomFrameSplitter(TcpCustomFrameSplitter splitter)
{
    m_customSplitter = std::move(splitter);
    if (m_started)
        pushSplitterToWorker();
}

void TcpClient::clearReceiveBuffer()
{
    if (!m_started || !m_worker)
        return;
    QMetaObject::invokeMethod(m_worker, "slotClearReceiveBuffer", Qt::QueuedConnection);
}

int TcpClient::receiveBufferSize() const
{
    if (!m_worker)
        return 0;
    int sz = 0;
    QMetaObject::invokeMethod(
        m_worker,
        [w = m_worker, &sz]() { sz = w->receiveBufferSize(); },
        Qt::BlockingQueuedConnection);
    return sz;
}
