#ifndef TCP_PEER_TEST_MAINWINDOW_H
#define TCP_PEER_TEST_MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief P2 真对端：独立进程 TCP Server，配合 TcpUiTest 客户端联调。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onListen();
    void onStop();
    void onClearLog();
    void onManualSend();
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    enum class LogKind { Info, Step, Ok, Warn, Err, Rx, Tx };

    void appendLog(LogKind kind, const QString& line);
    void stopAll();
    void handleData(QTcpSocket* sock, const QByteArray& data);
    QByteArray parsePayload(bool* ok, QString* err) const;
    static QString bytesToPreview(const QByteArray& data);
    static QString escapeHtml(const QString& s);

    Ui::MainWindow* ui = nullptr;
    QTcpServer m_server;
    QList<QTcpSocket*> m_clients;
};

#endif // TCP_PEER_TEST_MAINWINDOW_H
