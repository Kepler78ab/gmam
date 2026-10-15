#ifndef MODULE_TCP_TCPCLIENTWORKER_H
#define MODULE_TCP_TCPCLIENTWORKER_H

#include "TcpClientConfig.h"
#include "SendQueue.h"
#include "FrameAssembler.h"
#include "IFrameMatcher.h"

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QElapsedTimer>
#include <atomic>

/**
 * @file TcpClientWorker.h
 * @brief 工作线程对象：QTcpSocket、发送队列、重连、心跳、wait-ack、拆帧。
 * @note 仅由 TcpClient 通过 queued/blocking 调用；应用勿直接实例化使用。
 */
class TcpClientWorker : public QObject {
    Q_OBJECT
public:
    explicit TcpClientWorker(QObject* parent = nullptr);
    ~TcpClientWorker() override;

    /** @brief 发送队列指针（门面侧 enqueue 直接操作）。 */
    SendQueue* sendQueue() { return &m_sendQueue; }

    /**
     * @brief 是否已连接（原子读）。
     * @return true 已连接
     */
    bool isConnectedFlag() const { return m_connected.load(); }

    /**
     * @brief 设置 A/N/心跳忽略 matcher（须在 worker 线程调用）。
     */
    void setAckMatchers(TcpFrameMatcher positive,
                        TcpFrameMatcher negative,
                        TcpFrameMatcher heartbeatIgnore);

    /** @brief 设置心跳帧 provider。 */
    void setHeartbeatProvider(TcpHeartbeatFrameProvider provider);

    /** @brief 设置 Custom 拆帧回调。 */
    void setCustomSplitter(TcpCustomFrameSplitter splitter);

public slots:
    /** @brief 应用配置；PesAscii 会映射为 LengthPrefix 参数。 */
    void slotApplyConfig(TcpClientConfig config);
    /** @brief 发起连接。 */
    void slotConnect();
    /**
     * @brief 断开。
     * @param userRequested true=用户主动断开，不再自动重连
     */
    void slotDisconnect(bool userRequested);
    /** @brief 尝试从队列泵出发送（透传或 wait-ack）。 */
    void slotPumpSend();
    /**
     * @brief 根据队列长度检查水位信号。
     * @param size 当前队列长度
     */
    void slotNotifyQueueSize(int size);
    /** @brief 关闭 socket、停定时器、清空队列。 */
    void slotShutdown();

signals:
    void sigConnected();                                    ///< 已连接
    void sigDisconnected();                                 ///< 已断开
    void sigError(const QString& message);                  ///< 错误文案
    void sigBytesReceived(const QByteArray& data);          ///< 原始 readAll
    void sigFrameReceived(const QByteArray& frame);         ///< 完整帧
    void sigQueueRejected();                                ///< 队列拒收（门面也可直接发）
    void sigQueueHighWater();                               ///< 高水位
    void sigQueueLowWater();                                ///< 低水位
    void sigSendAccepted();                                 ///< 正应答
    void sigSendRejected(const QString& reason);            ///< 负应答或超时

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onReconnectTimeout();
    void onConnectTimeout();
    void onHeartbeatTimeout();
    void onAckTimeout();
    void onSendGapTimeout();

private:
    void ensureSocket();
    void ensureStrategyTimers();
    void startReconnectTimer();
    void stopReconnectTimer();
    void abortConnecting();
    void writeOne(const QByteArray& frame);
    void updateHeartbeatTimer();
    void scheduleSendGap();
    void clearInFlight();
    void dispatchFrame(const QByteArray& frame);
    void onAckPositive();
    void onAckNegative(const QString& reason);
    void pumpPassThrough();
    void pumpWithAck();

    TcpClientConfig m_config;
    SendQueue m_sendQueue;
    FrameAssembler m_assembler;
    QTcpSocket* m_socket = nullptr;
    QTimer* m_reconnectTimer = nullptr;
    QTimer* m_connectTimer = nullptr;
    QTimer* m_heartbeatTimer = nullptr;
    QTimer* m_ackTimer = nullptr;
    QTimer* m_sendGapTimer = nullptr;
    std::atomic<bool> m_connected{false};
    bool m_userDisconnect = false; ///< 用户主动断开
    bool m_wantConnected = false;  ///< 是否仍希望保持连接（用于重连）

    TcpFrameMatcher m_ackPositive;
    TcpFrameMatcher m_ackNegative;
    TcpFrameMatcher m_heartbeatIgnore;
    TcpHeartbeatFrameProvider m_heartbeatProvider;

    // wait-ack 停等窗口（窗口=1）
    bool m_inFlight = false;
    QByteArray m_inFlightFrame;
    int m_inFlightSendCount = 0;

    bool m_highWaterEmitted = false;
    QElapsedTimer m_lastSendTimer;
    bool m_lastSendValid = false;
};

#endif // MODULE_TCP_TCPCLIENTWORKER_H
