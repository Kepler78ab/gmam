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
 * @brief 工作线程：socket + 队列 + 重连 +（P1）心跳/ack/拆帧
 */
class TcpClientWorker : public QObject {
    Q_OBJECT
public:
    explicit TcpClientWorker(QObject* parent = nullptr);
    ~TcpClientWorker() override;

    SendQueue* sendQueue() { return &m_sendQueue; }
    bool isConnectedFlag() const { return m_connected.load(); }

    void setAckMatchers(TcpFrameMatcher positive,
                        TcpFrameMatcher negative,
                        TcpFrameMatcher heartbeatIgnore);
    void setHeartbeatProvider(TcpHeartbeatFrameProvider provider);
    void setCustomSplitter(TcpCustomFrameSplitter splitter);

public slots:
    void slotApplyConfig(TcpClientConfig config);
    void slotConnect();
    void slotDisconnect(bool userRequested);
    void slotPumpSend();
    void slotNotifyQueueSize(int size);
    void slotShutdown();

signals:
    void sigConnected();
    void sigDisconnected();
    void sigError(const QString& message);
    void sigBytesReceived(const QByteArray& data);
    void sigFrameReceived(const QByteArray& frame);
    void sigQueueRejected();
    void sigQueueHighWater();
    void sigQueueLowWater();
    void sigSendAccepted();
    void sigSendRejected(const QString& reason);

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
    bool m_userDisconnect = false;
    bool m_wantConnected = false;

    TcpFrameMatcher m_ackPositive;
    TcpFrameMatcher m_ackNegative;
    TcpFrameMatcher m_heartbeatIgnore;
    TcpHeartbeatFrameProvider m_heartbeatProvider;

    bool m_inFlight = false;
    QByteArray m_inFlightFrame;
    int m_inFlightSendCount = 0;

    bool m_highWaterEmitted = false;
    QElapsedTimer m_lastSendTimer;
    bool m_lastSendValid = false;
};

#endif // MODULE_TCP_TCPCLIENTWORKER_H
