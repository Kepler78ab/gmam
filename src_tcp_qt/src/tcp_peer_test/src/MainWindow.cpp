#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QAbstractSocket>
#include <QDateTime>
#include <QHostAddress>
#include <QMessageBox>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnListen, &QPushButton::clicked, this, &MainWindow::onListen);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(ui->btnClearLog, &QPushButton::clicked, this, &MainWindow::onClearLog);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::onManualSend);
    connect(&m_server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    appendLog(LogKind::Step, QStringLiteral("========== 真对端 TcpPeerTest =========="));
    appendLog(LogKind::Step, QStringLiteral("① 本窗口点【开始监听】（默认 127.0.0.1:9000）"));
    appendLog(LogKind::Step, QStringLiteral("② 另开 TcpUiTest 工程，连同一 IP:端口"));
    appendLog(LogKind::Step, QStringLiteral("③ 在客户端点发送；本窗口看收包并自动/手动回复"));
    appendLog(LogKind::Info, QStringLiteral("紫=收包  蓝=回包  红=错误"));
}

MainWindow::~MainWindow()
{
    stopAll();
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
    case LogKind::Rx:
        tag = QStringLiteral("[对端←收]");
        color = QStringLiteral("#6A1B9A");
        break;
    case LogKind::Tx:
        tag = QStringLiteral("[对端→发]");
        color = QStringLiteral("#0057D9");
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

QByteArray MainWindow::parsePayload(bool* ok, QString* err) const
{
    *ok = true;
    err->clear();
    const QString raw = ui->editSend->toPlainText();
    if (!ui->chkHex->isChecked())
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
        *err = QStringLiteral("内容为空");
    }
    return out;
}

void MainWindow::stopAll()
{
    for (QTcpSocket* s : m_clients) {
        if (!s)
            continue;
        s->disconnect(this);
        s->abort();
        s->deleteLater();
    }
    m_clients.clear();
    if (m_server.isListening())
        m_server.close();
    ui->lblStatus->setText(QStringLiteral("未监听"));
}

void MainWindow::onListen()
{
    if (m_server.isListening()) {
        appendLog(LogKind::Warn,
                  QStringLiteral("已在监听 %1:%2")
                      .arg(m_server.serverAddress().toString())
                      .arg(m_server.serverPort()));
        return;
    }

    QHostAddress addr(ui->editHost->text().trimmed());
    if (addr.isNull())
        addr = QHostAddress::LocalHost;
    const quint16 port = static_cast<quint16>(ui->spinPort->value());

    if (!m_server.listen(addr, port)) {
        appendLog(LogKind::Err, QStringLiteral("监听失败：%1").arg(m_server.errorString()));
        QMessageBox::warning(this, QStringLiteral("监听失败"), m_server.errorString());
        return;
    }

    ui->lblStatus->setText(QStringLiteral("● 监听中 :%1").arg(m_server.serverPort()));
    appendLog(LogKind::Ok,
              QStringLiteral("开始监听 %1:%2 | 收到后=%3")
                  .arg(m_server.serverAddress().toString())
                  .arg(m_server.serverPort())
                  .arg(ui->comboMode->currentText()));
    appendLog(LogKind::Step, QStringLiteral("请打开另一个工程 TcpUiTest，连到同一地址端口"));
}

void MainWindow::onStop()
{
    const bool was = m_server.isListening() || !m_clients.isEmpty();
    stopAll();
    if (was)
        appendLog(LogKind::Warn, QStringLiteral("已停止监听并断开全部客户端"));
    else
        appendLog(LogKind::Info, QStringLiteral("本来就没在听"));
}

void MainWindow::onClearLog()
{
    ui->textLog->clear();
    appendLog(LogKind::Info, QStringLiteral("日志已清空"));
}

void MainWindow::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket* sock = m_server.nextPendingConnection();
        m_clients.append(sock);
        connect(sock, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        connect(sock, &QTcpSocket::disconnected, this, &MainWindow::onClientDisconnected);
        connect(sock,
                QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this,
                &MainWindow::onSocketError);
        appendLog(LogKind::Ok,
                  QStringLiteral("客户端连入 %1:%2 | 当前连接数=%3")
                      .arg(sock->peerAddress().toString())
                      .arg(sock->peerPort())
                      .arg(m_clients.size()));
    }
}

void MainWindow::onReadyRead()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock)
        return;
    const QByteArray data = sock->readAll();
    if (!data.isEmpty())
        handleData(sock, data);
}

void MainWindow::handleData(QTcpSocket* sock, const QByteArray& data)
{
    appendLog(LogKind::Rx,
              QStringLiteral("来自 %1:%2 → %3")
                  .arg(sock->peerAddress().toString())
                  .arg(sock->peerPort())
                  .arg(bytesToPreview(data)));

    const int mode = ui->comboMode->currentIndex();
    QByteArray reply;
    QString hint;
    if (mode == 0) {
        reply = QByteArray("echo:") + data;
        hint = QStringLiteral("Echo");
    } else if (mode == 1) {
        reply = ui->editAckPos->text().isEmpty() ? QByteArray("A") : ui->editAckPos->text().toUtf8();
        hint = QStringLiteral("正应答");
    } else if (mode == 2) {
        reply = ui->editAckNeg->text().isEmpty() ? QByteArray("N") : ui->editAckNeg->text().toUtf8();
        hint = QStringLiteral("负应答");
    } else {
        appendLog(LogKind::Warn, QStringLiteral("模式=只收不回，不自动回复"));
        return;
    }

    sock->write(reply);
    sock->flush();
    appendLog(LogKind::Tx, QStringLiteral("自动回复（%1）→ %2").arg(hint, bytesToPreview(reply)));
}

void MainWindow::onClientDisconnected()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock)
        return;
    m_clients.removeAll(sock);
    sock->deleteLater();
    appendLog(LogKind::Warn, QStringLiteral("有客户端断开，剩余连接数=%1").arg(m_clients.size()));
}

void MainWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock)
        return;
    appendLog(LogKind::Err, QStringLiteral("socket 错误：%1").arg(sock->errorString()));
}

void MainWindow::onManualSend()
{
    if (m_clients.isEmpty()) {
        appendLog(LogKind::Warn, QStringLiteral("没有已连接的客户端，无法发送"));
        return;
    }
    bool ok = false;
    QString err;
    const QByteArray payload = parsePayload(&ok, &err);
    if (!ok) {
        appendLog(LogKind::Err, err);
        QMessageBox::warning(this, QStringLiteral("发送失败"), err);
        return;
    }
    if (payload.isEmpty()) {
        appendLog(LogKind::Err, QStringLiteral("内容为空"));
        return;
    }
    int n = 0;
    for (QTcpSocket* s : m_clients) {
        if (!s || s->state() != QAbstractSocket::ConnectedState)
            continue;
        s->write(payload);
        s->flush();
        ++n;
    }
    appendLog(LogKind::Tx,
              QStringLiteral("手动发给 %1 个客户端 → %2").arg(n).arg(bytesToPreview(payload)));
}
