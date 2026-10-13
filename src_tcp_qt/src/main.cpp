#include "TcpClient.h"
#include "TcpClientConfig.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>

/**
 * P0 冒烟：本进程内 QTcpServer(127.0.0.1) + TcpClient 回环。
 */
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHost, 0)) {
        qWarning() << "server listen failed:" << server.errorString();
        return 1;
    }
    const quint16 port = server.serverPort();
    qInfo() << "smoke server listening on 127.0.0.1:" << port;

    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server]() {
        QTcpSocket* peer = server.nextPendingConnection();
        QObject::connect(peer, &QTcpSocket::readyRead, peer, [peer]() {
            const QByteArray in = peer->readAll();
            qInfo() << "[server] recv" << in;
            peer->write(QByteArray("echo:") + in);
            peer->flush();
        });
    });

    TcpClientConfig cfg = TcpClientConfig::defaults();
    cfg.host = QStringLiteral("127.0.0.1");
    cfg.port = port;
    cfg.autoReconnect = false;

    TcpClient client;
    client.setConfig(cfg);

    bool gotRecv = false;
    QObject::connect(&client, &TcpClient::connected, &client, [&]() {
        qInfo() << "[client] connected";
        client.enqueue(QByteArray("ping-p0"));
    });
    QObject::connect(&client, &TcpClient::bytesReceived, &client, [&](const QByteArray& data) {
        qInfo() << "[client] recv" << data;
        gotRecv = data.contains("echo:ping-p0");
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    });
    QObject::connect(&client, &TcpClient::errorOccurred, &client, [](const QString& e) {
        qWarning() << "[client] error" << e;
    });

    QTimer::singleShot(5000, &app, [&]() {
        qWarning() << "smoke timeout";
        app.quit();
    });

    client.connectToHost();
    const int rc = app.exec();
    client.disconnectFromHost();

    if (!gotRecv) {
        qWarning() << "GMAM/TCP P0 smoke FAILED";
        return 1;
    }
    qInfo() << "TCP P0 smoke PASSED";
    return rc;
}
