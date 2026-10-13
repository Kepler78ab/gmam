#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    connect(ui->btnApplyConfig, &QPushButton::clicked, this, &MainWindow::onApplyConfig);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::onSend);
    connect(ui->btnClearLog, &QPushButton::clicked, this, &MainWindow::onClearLog);

    connect(&m_client, &TcpClient::connected, this, &MainWindow::onClientConnected);
    connect(&m_client, &TcpClient::disconnected, this, &MainWindow::onClientDisconnected);
    connect(&m_client, &TcpClient::errorOccurred, this, &MainWindow::onClientError);
    connect(&m_client, &TcpClient::bytesReceived, this, &MainWindow::onBytesReceived);
    connect(&m_client, &TcpClient::frameReceived, this, &MainWindow::onFrameReceived);
    connect(&m_client, &TcpClient::queueRejected, this, &MainWindow::onQueueRejected);
    connect(&m_client, &TcpClient::queueHighWater, this, &MainWindow::onQueueHighWater);
    connect(&m_client, &TcpClient::queueLowWater, this, &MainWindow::onQueueLowWater);
    connect(&m_client, &TcpClient::sendAccepted, this, &MainWindow::onSendAccepted);
    connect(&m_client, &TcpClient::sendRejected, this, &MainWindow::onSendRejected);

    appendLog(LogKind::Step, QStringLiteral("========== 客户端 TcpUiTest =========="));
    appendLog(LogKind::Step, QStringLiteral("① 先打开并运行 TcpPeerTest.pro，点【开始监听】"));
    appendLog(LogKind::Step, QStringLiteral("② 本窗口用同一 IP:端口 点【连接】"));
    appendLog(LogKind::Step, QStringLiteral("③ 点【发送】；两边日志对照看收发"));
    appendLog(LogKind::Info, QStringLiteral("蓝=发  绿=收  红=错误  对端：tcp_peer_test/src/TcpPeerTest.pro"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::escapeHtml(const QString& s)
{
    QString out = s;
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    out.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    out.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    out.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    return out;
}

void MainWindow::appendLog(LogKind kind, const QString& line)
{
    QString tag;
    QString color;
    switch (kind) {
    case LogKind::Info:
        tag = QStringLiteral("[说明]");
        color = QStringLiteral("#444444");
        break;
    case LogKind::Step:
        tag = QStringLiteral("[步骤]");
        color = QStringLiteral("#0B5CAB");
        break;
    case LogKind::ClientTx:
        tag = QStringLiteral("[客户端→发]");
        color = QStringLiteral("#0057D9");
        break;
    case LogKind::ClientRx:
        tag = QStringLiteral("[客户端←收]");
        color = QStringLiteral("#1B7A1B");
        break;
    case LogKind::Ok:
        tag = QStringLiteral("[成功]");
        color = QStringLiteral("#0A7A0A");
        break;
    case LogKind::Warn:
        tag = QStringLiteral("[告警]");
        color = QStringLiteral("#B36B00");
        break;
    case LogKind::Err:
        tag = QStringLiteral("[错误]");
        color = QStringLiteral("#C62828");
        break;
    case LogKind::AckOk:
        tag = QStringLiteral("[ACK√ 正应答]");
        color = QStringLiteral("#00695C");
        break;
    case LogKind::AckFail:
        tag = QStringLiteral("[ACK× 失败]");
        color = QStringLiteral("#AD1457");
        break;
    }

    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    const QString html = QStringLiteral(
                             "<div style='color:%1;margin:1px 0;'>"
                             "<b>%2</b> <span style='color:#888;'>%3</span> %4"
                             "</div>")
                             .arg(color, escapeHtml(tag), escapeHtml(ts), escapeHtml(line));
    ui->textLog->append(html);
    ui->textLog->verticalScrollBar()->setValue(ui->textLog->verticalScrollBar()->maximum());
}

void MainWindow::logConfigSnapshot(const QString& title, const TcpClientConfig& cfg)
{
    appendLog(LogKind::Info, QStringLiteral("----- %1 -----").arg(title));
    appendLog(LogKind::Info,
              QStringLiteral("目标 %1:%2 | 自动重连=%3")
                  .arg(cfg.host)
                  .arg(cfg.port)
                  .arg(cfg.autoReconnect ? QStringLiteral("开") : QStringLiteral("关")));
    appendLog(LogKind::Info,
              QStringLiteral("心跳=%1 间隔=%2ms 帧=\"%3\" | wait-ack=%4 超时=%5ms 最大发送=%6 | 拆帧=%7")
                  .arg(cfg.heartbeatEnabled ? QStringLiteral("开") : QStringLiteral("关"))
                  .arg(cfg.heartbeatIntervalMs)
                  .arg(QString::fromUtf8(cfg.heartbeatFrame))
                  .arg(cfg.ackEnabled ? QStringLiteral("开") : QStringLiteral("关"))
                  .arg(cfg.ackTimeoutMs)
                  .arg(cfg.ackMaxRetry)
                  .arg(ui->comboFrameMode->currentText()));
    appendLog(LogKind::Info,
              QStringLiteral("正应答匹配含 \"%1\" | 负应答匹配含 \"%2\"")
                  .arg(ui->editAckPos->text(), ui->editAckNeg->text()));
}

QString MainWindow::bytesToPreview(const QByteArray& data)
{
    QString hex = QString::fromLatin1(data.toHex(' ')).toUpper();
    QString text = QString::fromUtf8(data);
    for (QChar& c : text) {
        if (!c.isPrint() && c != QLatin1Char('\n') && c != QLatin1Char('\r') && c != QLatin1Char('\t'))
            c = QLatin1Char('.');
    }
    if (text.size() > 200)
        text = text.left(200) + QStringLiteral("…");
    if (hex.size() > 240)
        hex = hex.left(240) + QStringLiteral("…");
    return QStringLiteral("%1 字节 | 文本「%2」 | HEX %3").arg(data.size()).arg(text, hex);
}

TcpClientConfig MainWindow::buildConfigFromUi() const
{
    TcpClientConfig cfg = TcpClientConfig::defaults();
    cfg.host = ui->editHost->text().trimmed();
    if (cfg.host.isEmpty())
        cfg.host = QStringLiteral("127.0.0.1");
    cfg.port = static_cast<quint16>(ui->spinPort->value());
    cfg.autoReconnect = ui->chkAutoReconnect->isChecked();

    cfg.heartbeatEnabled = ui->chkHeartbeat->isChecked();
    cfg.heartbeatIntervalMs = ui->spinHbInterval->value();
    if (ui->chkHbUseSendBox->isChecked())
        cfg.heartbeatFrame = ui->editSend->toPlainText().toUtf8();
    else
        cfg.heartbeatFrame = ui->editHbFrame->text().toUtf8();

    cfg.ackEnabled = ui->chkAck->isChecked();
    cfg.ackTimeoutMs = ui->spinAckTimeout->value();
    cfg.ackMaxRetry = ui->spinAckMaxRetry->value();

    const int mode = ui->comboFrameMode->currentIndex();
    if (mode == 1)
        cfg.frameMode = TcpFrameMode::LengthPrefix;
    else if (mode == 2)
        cfg.frameMode = TcpFrameMode::PesAscii;
    else
        cfg.frameMode = TcpFrameMode::Raw;

    return cfg;
}

void MainWindow::applyMatchersAndHeartbeat()
{
    const QByteArray posNeedle = ui->editAckPos->text().toUtf8();
    const QByteArray negNeedle = ui->editAckNeg->text().toUtf8();

    TcpFrameMatcher pos = [posNeedle](const QByteArray& f) {
        return !posNeedle.isEmpty() && f.contains(posNeedle);
    };
    TcpFrameMatcher neg = [negNeedle](const QByteArray& f) {
        return !negNeedle.isEmpty() && f.contains(negNeedle);
    };
    m_client.setAckMatchers(pos, neg);

    if (ui->chkHeartbeat->isChecked()) {
        const QByteArray hb = ui->chkHbUseSendBox->isChecked()
                                  ? ui->editSend->toPlainText().toUtf8()
                                  : ui->editHbFrame->text().toUtf8();
        m_client.setHeartbeatFrame(hb);
    }
}

QByteArray MainWindow::parseSendPayload(bool* ok, QString* err) const
{
    *ok = true;
    err->clear();
    const QString raw = ui->editSend->toPlainText();
    if (!ui->chkHexSend->isChecked())
        return raw.toUtf8();

    QByteArray out;
    QString token;
    auto flushToken = [&]() -> bool {
        if (token.isEmpty())
            return true;
        bool conv = false;
        const int v = token.toInt(&conv, 16);
        token.clear();
        if (!conv || v < 0 || v > 255) {
            *ok = false;
            *err = QStringLiteral("非法十六进制");
            return false;
        }
        out.append(static_cast<char>(v));
        return true;
    };
    for (const QChar c : raw) {
        if (c.isSpace()) {
            if (!flushToken())
                return {};
            continue;
        }
        token.append(c);
    }
    if (!flushToken())
        return {};
    if (out.isEmpty()) {
        *ok = false;
        *err = QStringLiteral("十六进制内容为空");
    }
    return out;
}

void MainWindow::onConnect()
{
    applyMatchersAndHeartbeat();
    const TcpClientConfig cfg = buildConfigFromUi();
    m_client.setConfig(cfg);
    logConfigSnapshot(QStringLiteral("即将连接"), cfg);
    appendLog(LogKind::Step,
              QStringLiteral("正在连接 %1:%2 …（请确认 TcpPeerTest 已在同端口监听）")
                  .arg(cfg.host)
                  .arg(cfg.port));
    ui->lblStatus->setText(QStringLiteral("连接中…"));
    m_client.connectToHost();
}

void MainWindow::onDisconnect()
{
    appendLog(LogKind::Step, QStringLiteral("主动请求断开"));
    m_client.disconnectFromHost();
}

void MainWindow::onApplyConfig()
{
    applyMatchersAndHeartbeat();
    const TcpClientConfig cfg = buildConfigFromUi();
    m_client.setConfig(cfg);
    logConfigSnapshot(QStringLiteral("已应用配置（不断开）"), cfg);
}

void MainWindow::onSend()
{
    if (!m_client.isConnected()) {
        appendLog(LogKind::Warn, QStringLiteral("尚未连接成功，仍会尝试入队（连上后可能发出）"));
    }
    bool ok = false;
    QString err;
    const QByteArray payload = parseSendPayload(&ok, &err);
    if (!ok) {
        appendLog(LogKind::Err, QStringLiteral("发送内容解析失败：%1").arg(err));
        QMessageBox::warning(this, QStringLiteral("发送失败"), err);
        return;
    }
    if (payload.isEmpty()) {
        appendLog(LogKind::Err, QStringLiteral("发送内容为空"));
        QMessageBox::warning(this, QStringLiteral("发送失败"), QStringLiteral("内容为空"));
        return;
    }
    if (!m_client.enqueue(payload)) {
        appendLog(LogKind::Err, QStringLiteral("入队失败：队列满被拒绝，或未启用队列且当前未连接"));
        return;
    }
    appendLog(LogKind::ClientTx, QStringLiteral("已入队发送 → %1").arg(bytesToPreview(payload)));
    if (ui->chkAck->isChecked()) {
        appendLog(LogKind::Info,
                  QStringLiteral("wait-ack 已开：等对端回含「%1」才算成功，含「%2」则失败")
                      .arg(ui->editAckPos->text(), ui->editAckNeg->text()));
    }
}

void MainWindow::onClearLog()
{
    ui->textLog->clear();
    appendLog(LogKind::Info, QStringLiteral("日志已清空"));
}

void MainWindow::onClientConnected()
{
    ui->lblStatus->setText(QStringLiteral("● 已连接"));
    appendLog(LogKind::Ok, QStringLiteral("连接成功，可以发送"));
}

void MainWindow::onClientDisconnected()
{
    ui->lblStatus->setText(QStringLiteral("未连接"));
    appendLog(LogKind::Warn, QStringLiteral("已断开"));
}

void MainWindow::onClientError(const QString& message)
{
    appendLog(LogKind::Err, QStringLiteral("%1").arg(message));
}

void MainWindow::onBytesReceived(const QByteArray& data)
{
    appendLog(LogKind::ClientRx,
              QStringLiteral("原始字节 → %1").arg(bytesToPreview(data)));
}

void MainWindow::onFrameReceived(const QByteArray& frame)
{
    appendLog(LogKind::ClientRx,
              QStringLiteral("完整帧 → %1").arg(bytesToPreview(frame)));
}

void MainWindow::onQueueRejected()
{
    appendLog(LogKind::Err, QStringLiteral("发送队列已满，本帧被拒绝"));
}

void MainWindow::onQueueHighWater()
{
    appendLog(LogKind::Warn, QStringLiteral("发送队列高水位"));
}

void MainWindow::onQueueLowWater()
{
    appendLog(LogKind::Info, QStringLiteral("发送队列低水位"));
}

void MainWindow::onSendAccepted()
{
    appendLog(LogKind::AckOk,
              QStringLiteral("正应答匹配成功（含「%1」）").arg(ui->editAckPos->text()));
}

void MainWindow::onSendRejected(const QString& reason)
{
    appendLog(LogKind::AckFail, QStringLiteral("%1").arg(reason));
}
