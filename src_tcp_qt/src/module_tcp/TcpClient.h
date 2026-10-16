#ifndef MODULE_TCP_TCPCLIENT_H
#define MODULE_TCP_TCPCLIENT_H

#include "TcpClientConfig.h"
#include "IFrameMatcher.h"

#include <QObject>
#include <QThread>

class TcpClientWorker;

/**
 * @file TcpClient.h
 * @brief module_tcp 门面：主线程调用，内部工作线程跑 socket / 队列 / 策略。
 *
 * 组报不在本模块；传入已组好的 QByteArray。调用说明见 docs/使用说明.md。
 */
class TcpClient : public QObject {
    Q_OBJECT
public:
    /**
     * @brief 构造客户端（尚未启动工作线程）。
     * @param parent Qt 父对象
     */
    explicit TcpClient(QObject* parent = nullptr);

    /**
     * @brief 析构：关闭 worker 并停止工作线程。
     */
    ~TcpClient() override;

    /**
     * @brief 写入运行配置。
     * @param config 地址、队列、心跳/ack 开关、拆帧等；matcher 等不在此结构内
     */
    void setConfig(const TcpClientConfig& config);

    /**
     * @brief 读取当前门面侧保存的配置副本。
     */
    TcpClientConfig config() const;

    /**
     * @brief 按当前配置连接远端（必要时启动工作线程）。
     * @note 若 ackEnabled，请先 setAckMatchers
     */
    void connectToHost();

    /**
     * @brief 主动断开连接（用户断开后默认不再自动重连，直至再次 connectToHost）。
     */
    void disconnectFromHost();

    /**
     * @brief 当前是否已连接。
     * @return true 已连接
     */
    bool isConnected() const;

    /**
     * @brief 入队发送一整帧。
     * @param frame 已组好的完整帧；空帧不会入队
     * @return true 已入队；false 空帧、Reject 下队列满、或未开队列且未连接
     */
    bool enqueue(const QByteArray& frame);

    /**
     * @brief 设置固定心跳整帧（同时写入 config.heartbeatFrame）。
     * @param frame 心跳内容；需 heartbeatEnabled=true 才会周期发送
     * @note 若同时设置了 provider，则以 provider 为准
     */
    void setHeartbeatFrame(const QByteArray& frame);

    /**
     * @brief 设置动态心跳帧提供者。
     * @param provider 每次心跳定时触发时调用；返回空则本拍不发
     */
    void setHeartbeatFrameProvider(TcpHeartbeatFrameProvider provider);

    /**
     * @brief 设置应答判定回调。
     * @param positive 正应答：匹配则 sendAccepted，当前发送帧出队
     * @param negative 负应答：匹配则 sendRejected，当前发送帧丢弃
     * @param heartbeatIgnore 可选；匹配则本帧不驱动 ack（如对端心跳）
     * @note ackEnabled=true 时必须设置 positive 与 negative，否则会关 ack 并报错
     */
    void setAckMatchers(TcpFrameMatcher positive,
                        TcpFrameMatcher negative,
                        TcpFrameMatcher heartbeatIgnore = TcpFrameMatcher());

    /**
     * @brief 设置自定义拆帧函数。
     * @param splitter 从缓冲切完整帧；仅 frameMode==Custom 时生效
     */
    void setCustomFrameSplitter(TcpCustomFrameSplitter splitter);

    /**
     * @brief 清空接收侧拆帧缓冲与最近拆帧错误（异步投递到 worker，返回时未必已执行完）。
     * @note 不影响发送队列、wait-ack 窗口、TCP 连接状态。
     * @note ackEnabled 时：可能丢掉尚未参与 matcher 的应答字节，随后可能触发超时重传——属预期副作用。
     * @note 可靠协议级重同步仍由上层或 Custom splitter 负责。
     */
    void clearReceiveBuffer();

    /**
     * @brief 查询当前拆帧缓冲未消费字节数（跨线程快照，仅诊断用）。
     * @return 某一时刻的缓冲大小；worker 未启动时返回 0。勿作为强一致业务判定条件。
     */
    int receiveBufferSize() const;

signals:
    /** 连接成功。 */
    void connected();
    /** 连接断开。 */
    void disconnected();
    /**
     * 发生错误。
     * @param message 错误说明
     */
    void errorOccurred(const QString& message);
    /**
     * 本轮 socket readAll 的原始字节（可能粘包/半包）。
     * @param data 原始数据
     */
    void bytesReceived(const QByteArray& data);
    /**
     * 按拆帧规则得到的完整帧（业务优先使用）。
     * @param frame 完整帧
     */
    void frameReceived(const QByteArray& frame);
    /** enqueue 因队列满（Reject）未入队。 */
    void queueRejected();
    /** 队列长度达到/超过 highWaterMark（边沿）。 */
    void queueHighWater();
    /** 曾过高水位后回落到 lowWaterMark 及以下。 */
    void queueLowWater();
    /** wait-ack：正应答匹配，当前发送帧已确认。 */
    void sendAccepted();
    /**
     * wait-ack：负应答或超时达 ackMaxRetry。
     * @param reason 原因文案
     */
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
