#ifndef MODULE_TCP_TCPTYPES_H
#define MODULE_TCP_TCPTYPES_H

#include <QString>

/**
 * @file TcpTypes.h
 * @brief module_tcp 公共枚举（P0；P1 字段已预留）
 */

/** 收包模式 */
enum class TcpFrameMode {
    Raw = 0,           ///< P0：readAll 原样上抛
    LengthPrefix = 1,  ///< P1
    Custom = 2,        ///< P1
    PesAscii = 3       ///< P1 可选预设
};

/** 发送队列满时策略 */
enum class TcpQueueOverflowPolicy {
    Reject = 0,      ///< P0：拒收
    DropOldest = 1   ///< P1
};

/** 工作线程模式 */
enum class TcpWorkerThreadMode {
    Dedicated = 0, ///< 模块内独立线程（P0 默认）
    Caller = 1     ///< 其后：由调用方 moveToThread
};

/** 长度前缀编码（P1） */
enum class TcpLengthPrefixEncoding {
    DecimalAscii = 0,
    BinaryBE = 1,
    BinaryLE = 2
};

#endif // MODULE_TCP_TCPTYPES_H
