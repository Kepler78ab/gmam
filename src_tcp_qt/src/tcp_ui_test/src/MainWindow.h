#ifndef TCP_UI_TEST_MAINWINDOW_H
#define TCP_UI_TEST_MAINWINDOW_H

#include "TcpClient.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief P2 客户端：配合独立工程 TcpPeerTest 真对端联调。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnect();
    void onDisconnect();
    void onApplyConfig();
    void onSend();
    void onClearLog();

    void onClientConnected();
    void onClientDisconnected();
    void onClientError(const QString& message);
    void onBytesReceived(const QByteArray& data);
    void onFrameReceived(const QByteArray& frame);
    void onQueueRejected();
    void onQueueHighWater();
    void onQueueLowWater();
    void onSendAccepted();
    void onSendRejected(const QString& reason);

private:
    enum class LogKind {
        Info,
        Step,
        ClientTx,
        ClientRx,
        Ok,
        Warn,
        Err,
        AckOk,
        AckFail
    };

    void appendLog(LogKind kind, const QString& line);
    void logConfigSnapshot(const QString& title, const TcpClientConfig& cfg);
    TcpClientConfig buildConfigFromUi() const;
    void applyMatchersAndHeartbeat();
    QByteArray parseSendPayload(bool* ok, QString* err) const;
    static QString bytesToPreview(const QByteArray& data);
    static QString escapeHtml(const QString& s);

    Ui::MainWindow* ui = nullptr;
    TcpClient m_client;
};

#endif // TCP_UI_TEST_MAINWINDOW_H
