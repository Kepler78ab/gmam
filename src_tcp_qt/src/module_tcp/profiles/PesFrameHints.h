#ifndef MODULE_TCP_PROFILES_PESFRAMEHINTS_H
#define MODULE_TCP_PROFILES_PESFRAMEHINTS_H

/**
 * @file PesFrameHints.h
 * @brief 可选 PES/旧 toPes 协议辅助：拆帧字段建议值 + 简易 A/N/心跳 matcher。
 *
 * 不写死进 TcpClient 核心；应用按需 include 使用。
 */

#include "IFrameMatcher.h"
#include "TcpClientConfig.h"

#include <QByteArray>

namespace tcp_pes {

/** 现行 STRU_MSG_HEAD 中 msgType 字节偏移。 */
inline constexpr int kMsgTypeOffset = 21;

/** 心跳电文号区常见哨兵数字字符。 */
inline constexpr char kHeartbeatMsgNumDigit = '9';

/**
 * @brief 将配置填成 PesAscii 拆帧（6 位十进制总长 + 尾 0x0A）。
 * @param cfg 原配置
 * @return 填好拆帧相关字段后的配置
 */
inline TcpClientConfig applyPesFramingHints(TcpClientConfig cfg)
{
    cfg.frameMode = TcpFrameMode::PesAscii;
    cfg.lengthPrefixOffset = 0;
    cfg.lengthPrefixSize = 6;
    cfg.lengthPrefixEncoding = TcpLengthPrefixEncoding::DecimalAscii;
    cfg.lengthIncludesHeader = true;
    cfg.hasFrameTerminator = true;
    cfg.frameTerminator = static_cast<char>(0x0A);
    return cfg;
}

/**
 * @brief 在 PesAscii 拆帧基础上打开常用心跳/ack 开关（不含 matcher 与心跳帧内容）。
 * @param cfg 原配置
 * @return 建议运行标志后的配置；仍须 setAckMatchers / setHeartbeatFrame
 */
inline TcpClientConfig recommendedRuntimeFlags(TcpClientConfig cfg)
{
    cfg = applyPesFramingHints(std::move(cfg));
    cfg.heartbeatEnabled = true;
    cfg.heartbeatIntervalMs = 5000;
    cfg.heartbeatExpectReply = false;
    cfg.ackEnabled = true;
    cfg.ackTimeoutMs = 2000;
    cfg.ackMaxRetry = 5;
    return cfg;
}

/**
 * @brief 粗判是否像心跳帧（电文号区含连续 9）。
 * @param frame 完整帧
 * @return true 像心跳
 */
inline bool looksLikeHeartbeat(const QByteArray& frame)
{
    // 头-only 约 23：电文号区（offset 11, len 10）多为 9
    if (frame.size() < 22)
        return false;
    const QByteArray num = frame.mid(11, 10);
    return num.contains("9999999999");
}

/**
 * @brief 正应答 matcher：偏移 21 为 'A'（心跳帧返回 false）。
 */
inline TcpFrameMatcher matcherPositiveA()
{
    return [](const QByteArray& frame) {
        if (looksLikeHeartbeat(frame) || frame.size() <= kMsgTypeOffset)
            return false;
        return frame.at(kMsgTypeOffset) == 'A';
    };
}

/**
 * @brief 负应答 matcher：偏移 21 为 'N'（心跳帧返回 false）。
 */
inline TcpFrameMatcher matcherNegativeN()
{
    return [](const QByteArray& frame) {
        if (looksLikeHeartbeat(frame) || frame.size() <= kMsgTypeOffset)
            return false;
        return frame.at(kMsgTypeOffset) == 'N';
    };
}

/**
 * @brief 心跳忽略 matcher：像心跳则不驱动 wait-ack。
 */
inline TcpFrameMatcher matcherHeartbeatIgnore()
{
    return [](const QByteArray& frame) { return looksLikeHeartbeat(frame); };
}

} // namespace tcp_pes

#endif // MODULE_TCP_PROFILES_PESFRAMEHINTS_H
