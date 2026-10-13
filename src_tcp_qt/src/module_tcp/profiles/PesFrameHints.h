#ifndef MODULE_TCP_PROFILES_PESFRAMEHINTS_H
#define MODULE_TCP_PROFILES_PESFRAMEHINTS_H

/**
 * @file PesFrameHints.h
 * @brief PES/旧 toPes 协议辅助（可选）：拆帧建议值 + 简易 matcher
 *
 * 不写死进 TcpClient 核心；应用可选用。
 */

#include "IFrameMatcher.h"
#include "TcpClientConfig.h"

#include <QByteArray>

namespace tcp_pes {

/** 现行 STRU_MSG_HEAD：msgType 在偏移 21 */
inline constexpr int kMsgTypeOffset = 21;
/** 心跳电文号哨兵（10 字节区常见） */
inline constexpr char kHeartbeatMsgNumDigit = '9';

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

inline bool looksLikeHeartbeat(const QByteArray& frame)
{
    // 头-only 约 23：电文号区（offset 11, len 10）多为 9
    if (frame.size() < 22)
        return false;
    const QByteArray num = frame.mid(11, 10);
    return num.contains("9999999999");
}

inline TcpFrameMatcher matcherPositiveA()
{
    return [](const QByteArray& frame) {
        if (looksLikeHeartbeat(frame) || frame.size() <= kMsgTypeOffset)
            return false;
        return frame.at(kMsgTypeOffset) == 'A';
    };
}

inline TcpFrameMatcher matcherNegativeN()
{
    return [](const QByteArray& frame) {
        if (looksLikeHeartbeat(frame) || frame.size() <= kMsgTypeOffset)
            return false;
        return frame.at(kMsgTypeOffset) == 'N';
    };
}

inline TcpFrameMatcher matcherHeartbeatIgnore()
{
    return [](const QByteArray& frame) { return looksLikeHeartbeat(frame); };
}

} // namespace tcp_pes

#endif // MODULE_TCP_PROFILES_PESFRAMEHINTS_H
