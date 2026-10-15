#ifndef MODULE_TCP_IFRAMEMATCHER_H
#define MODULE_TCP_IFRAMEMATCHER_H

#include <QByteArray>
#include <QVector>
#include <functional>

/**
 * @file IFrameMatcher.h
 * @brief 帧判定与自定义拆帧的回调类型。
 */

/**
 * @brief 完整帧判定（正/负应答、心跳忽略等）。
 * @param frame 已拆好的完整帧
 * @return true 表示匹配该判定
 */
using TcpFrameMatcher = std::function<bool(const QByteArray& frame)>;

/**
 * @brief 自定义拆帧：从累积缓冲中取出完整帧。
 * @param buffer 模块已 append 新数据；调用方须删掉已消费字节，半包留下
 * @return 本轮拆出的完整帧列表（可空、可多帧）
 * @note 仅 frameMode == Custom 时使用；未设置则不出帧
 */
using TcpCustomFrameSplitter = std::function<QVector<QByteArray>(QByteArray& buffer)>;

#endif // MODULE_TCP_IFRAMEMATCHER_H
