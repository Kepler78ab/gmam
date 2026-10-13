#ifndef MODULE_TCP_TCPCLIENT_H
#define MODULE_TCP_TCPCLIENT_H

#include "TcpClientConfig.h"
#include "IFrameMatcher.h"

#include <QObject>
#include <QThread>

class TcpClientWorker;

/**
 * @file TcpClient.h
 * @brief module_tcp 门面（P0+P1）
 */
class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);
    ~TcpClient() override;

    void setConfig(const TcpClientConfig& config);
    TcpClientConfig config() const;

    void connectToHost();
    void disconnectFromHost();
    bool isConnected() const;

    /**
     * 入队发送。
     * @return false：队列满被拒绝（Reject），或未启用队列且当前未连接
     */
    bool enqueue(const QByteArray& frame);

    /** P1：固定心跳帧（也可用 config.heartbeatFrame） */
    void setHeartbeatFrame(const QByteArray& frame);

    /** P1：每次心跳时取帧；优先于固定帧 */
    void setHeartbeatFrameProvider(TcpHeartbeatFrameProvider provider);

    /**
     * P1：应答判定。ackEnabled=true 时必须设置 A/N。
     * heartbeatIgnore 可选（匹配则不驱动 ack）。
     */
    void setAckMatchers(TcpFrameMatcher positive,
                        TcpFrameMatcher negative,
                        TcpFrameMatcher heartbeatIgnore = TcpFrameMatcher());

    /** P1：frameMode=Custom 时使用 */
    void setCustomFrameSplitter(TcpCustomFrameSplitter splitter);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& message);
    void bytesReceived(const QByteArray& data);
    void frameReceived(const QByteArray& frame);
    void queueRejected();
    void queueHighWater();
    void queueLowWater();
    void sendAccepted();
    void sendRejected(const QString& reason);

private:
    void ensureWorkerStarted();
    void pushMatchersToWorker();
    void pushProviderToWorker();
    void pushSplitterToWorker();

    TcpClientConfig m_config;
    QThread m_thread;
    TcpClientWorker* m_worker = nullptr;
    bool m_started = false;

    TcpFrameMatcher m_ackPositive;
    TcpFrameMatcher m_ackNegative;
    TcpFrameMatcher m_heartbeatIgnore;
    TcpHeartbeatFrameProvider m_heartbeatProvider;
    TcpCustomFrameSplitter m_customSplitter;
};

#endif // MODULE_TCP_TCPCLIENT_H
