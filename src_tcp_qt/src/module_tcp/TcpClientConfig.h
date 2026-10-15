#ifndef MODULE_TCP_TCPCLIENTCONFIG_H
#define MODULE_TCP_TCPCLIENTCONFIG_H

#include "TcpTypes.h"

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <functional>

/**
 * @file TcpClientConfig.h
 * @brief TcpClient 可拷贝配置。matcher / provider / splitter 不进本结构，经 TcpClient 接口设置。
 */

/**
 * @brief 客户端运行配置（可用 defaults() 取默认值）。
 */
struct TcpClientConfig {
    // ---- 连接 ----
    QString host = QStringLiteral("127.0.0.1"); ///< 远端主机名或 IP
    quint16 port = 9000;                       ///< 远端端口
    bool autoReconnect = true;                 ///< 断线后是否自动重连
    int reconnectIntervalMs = 3000;            ///< 重连间隔（毫秒）
    int connectTimeoutMs = 3000;               ///< 单次连接超时（毫秒）

    // ---- 发送队列 ----
    bool sendQueueEnabled = true; ///< 是否走发送队列（false 时需已连接才能发）
    int sendQueueMaxSize = 100;   ///< 队列最大帧数
    TcpQueueOverflowPolicy sendQueueOverflowPolicy = TcpQueueOverflowPolicy::Reject; ///< 满时策略

    // ---- 拆帧 ----
    TcpFrameMode frameMode = TcpFrameMode::Raw; ///< 收包拆帧模式，见 TcpFrameMode

    // ---- 线程 ----
    TcpWorkerThreadMode workerThreadMode = TcpWorkerThreadMode::Dedicated; ///< 工作线程模式（当前按 Dedicated）

    // ---- 水位 / 间隔 ----
    int sendIntervalMs = 0;   ///< 两帧最小间隔（毫秒）；0=不额外限速
    int highWaterMark = 100;  ///< 队列长度达此发 queueHighWater
    int lowWaterMark = 50;    ///< 过高水位后回落至此发 queueLowWater

    // ---- 心跳 ----
    bool heartbeatEnabled = false;     ///< 是否周期发送心跳
    int heartbeatIntervalMs = 5000;    ///< 心跳周期（毫秒）
    QByteArray heartbeatFrame;         ///< 固定心跳整帧；也可 TcpClient::setHeartbeatFrame
    bool heartbeatExpectReply = false; ///< 是否检测心跳回复（当前未实现检测逻辑）
    bool heartbeatOccupyAckWindow = false; ///< false=心跳旁路不占 wait-ack 槽
    int heartbeatMissLimit = 0;        ///< 心跳失联次数上限（占位，当前未用）

    // ---- wait-ack ----
    bool ackEnabled = false; ///< 是否停等应答再发下一帧
    int ackTimeoutMs = 2000; ///< 未收到 A/N 的超时（毫秒），超时则重发
    int ackMaxRetry = 5;     ///< 含首次在内的最大发送次数
    int ackWindow = 1;       ///< 同时未确认帧数（当前按 1 停等）

    // ---- LengthPrefix / PesAscii ----
    int lengthPrefixOffset = 0; ///< 长度字段起始偏移（字节）
    int lengthPrefixSize = 6;   ///< 长度字段字节数
    TcpLengthPrefixEncoding lengthPrefixEncoding = TcpLengthPrefixEncoding::DecimalAscii; ///< 长度编码
    bool lengthIncludesHeader = true; ///< true=长度即整帧长；false=长度为体长再加头等
    char frameTerminator = static_cast<char>(0x0A); ///< 帧结束符
    bool hasFrameTerminator = true; ///< 是否校验末字节为 frameTerminator
    int maxFrameSize = 8192; ///< 单帧最大字节；超长清空缓冲并报错

    /**
     * @brief 返回一份默认配置。
     */
    static TcpClientConfig defaults() { return TcpClientConfig{}; }
};

Q_DECLARE_METATYPE(TcpClientConfig)

/**
 * @brief 动态心跳帧提供者：每次心跳触发时调用。
 * @return 本拍要发送的心跳整帧；空则本拍不发
 */
using TcpHeartbeatFrameProvider = std::function<QByteArray()>;

#endif // MODULE_TCP_TCPCLIENTCONFIG_H
