#ifndef MODULE_TCP_TCPCLIENTCONFIG_H
#define MODULE_TCP_TCPCLIENTCONFIG_H

#include "TcpTypes.h"

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <functional>

/**
 * @file TcpClientConfig.h
 * @brief 可配置参数（P0+P1）。matcher/provider 通过 TcpClient 接口设置，不进本结构拷贝。
 * 默认值对齐 design_02 / design_03。
 */

struct TcpClientConfig {
    // ---- 链路（P0）----
    QString host = QStringLiteral("127.0.0.1");
    quint16 port = 9000;
    bool autoReconnect = true;
    int reconnectIntervalMs = 3000;
    int connectTimeoutMs = 3000;

    // ---- 发送队列（P0）----
    bool sendQueueEnabled = true;
    int sendQueueMaxSize = 100;
    TcpQueueOverflowPolicy sendQueueOverflowPolicy = TcpQueueOverflowPolicy::Reject;

    // ---- 接收（P0 仅 Raw）----
    TcpFrameMode frameMode = TcpFrameMode::Raw;

    // ---- 线程（P0）----
    TcpWorkerThreadMode workerThreadMode = TcpWorkerThreadMode::Dedicated;

    // ---- 队列水位 / 间隔（P1）----
    int sendIntervalMs = 0;
    int highWaterMark = 100;
    int lowWaterMark = 50;

    // ---- 心跳（P1）----
    bool heartbeatEnabled = false;
    int heartbeatIntervalMs = 5000;
    QByteArray heartbeatFrame;
    bool heartbeatExpectReply = false;
    bool heartbeatOccupyAckWindow = false;
    int heartbeatMissLimit = 0;

    // ---- wait-ack（P1）----
    bool ackEnabled = false;
    int ackTimeoutMs = 2000;
    int ackMaxRetry = 5;
    int ackWindow = 1;

    // ---- 拆帧（P1）----
    int lengthPrefixOffset = 0;
    int lengthPrefixSize = 6;
    TcpLengthPrefixEncoding lengthPrefixEncoding = TcpLengthPrefixEncoding::DecimalAscii;
    bool lengthIncludesHeader = true;
    char frameTerminator = static_cast<char>(0x0A);
    bool hasFrameTerminator = true;
    int maxFrameSize = 8192;

    static TcpClientConfig defaults() { return TcpClientConfig{}; }
};

Q_DECLARE_METATYPE(TcpClientConfig)

/** P1：心跳帧提供者（P0 不用） */
using TcpHeartbeatFrameProvider = std::function<QByteArray()>;

#endif // MODULE_TCP_TCPCLIENTCONFIG_H
