#ifndef MODULE_TCP_IFRAMEMATCHER_H
#define MODULE_TCP_IFRAMEMATCHER_H

#include <QByteArray>
#include <QVector>
#include <functional>

/**
 * @file IFrameMatcher.h
 * @brief 帧判定回调（P1）：A/N/心跳忽略、自定义拆帧
 */

/** @return true 表示匹配 */
using TcpFrameMatcher = std::function<bool(const QByteArray& frame)>;

/**
 * 自定义拆帧：从 buffer 中尽量取出完整帧，已消费部分从 buffer 移除。
 * @return 本次拆出的完整帧列表（可空）
 */
using TcpCustomFrameSplitter = std::function<QVector<QByteArray>(QByteArray& buffer)>;

#endif // MODULE_TCP_IFRAMEMATCHER_H
