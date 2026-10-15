#ifndef MODULE_TCP_TCPTYPES_H
#define MODULE_TCP_TCPTYPES_H

#include <QString>

/**
 * @file TcpTypes.h
 * @brief module_tcp 公共枚举。
 */

/**
 * @brief 收包拆帧模式。
 */
enum class TcpFrameMode {
    Raw = 0,           ///< 每次 readAll 的块即一帧（不攒缓冲）
    LengthPrefix = 1,  ///< 按长度前缀从缓冲拼完整帧
    Custom = 2,        ///< 由 setCustomFrameSplitter 自定义切帧
    PesAscii = 3       ///< 6 位十进制总长 + 尾 0x0A（内部按 LengthPrefix）
};

/**
 * @brief 发送队列已满时的策略。
 */
enum class TcpQueueOverflowPolicy {
    Reject = 0,      ///< 拒收本帧，enqueue 返回 false
    DropOldest = 1   ///< 丢弃最旧帧后再入队
};

/**
 * @brief 工作线程归属（当前实现按 Dedicated）。
 */
enum class TcpWorkerThreadMode {
    Dedicated = 0, ///< 模块内独立 QThread（默认）
    Caller = 1     ///< 预留：由调用方 moveToThread
};

/**
 * @brief 长度前缀字段的编码方式。
 */
enum class TcpLengthPrefixEncoding {
    DecimalAscii = 0, ///< ASCII 十进制数字（可含前导空格）
    BinaryBE = 1,     ///< 无符号整数，大端
    BinaryLE = 2      ///< 无符号整数，小端
};

#endif // MODULE_TCP_TCPTYPES_H
