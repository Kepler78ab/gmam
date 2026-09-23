#pragma once

#include <QByteArray>

/**
 * @file TelegramCompose.h
 * @brief 外层电文简单拼装：head + data + 结束符
 */

/**
 * 拼装完整电文字节流。
 * @param head        头部（可由 HeaderBuilder 产出）
 * @param data        数据内容（可由 DataContentBuilder 产出）
 * @param terminator  结束符，默认 0x0A
 * @return head + data + terminator
 *
 * @note 多层 / 多 schema：分别 build 后再 append，或多次调用本函数的变体自行拼接。
 */
QByteArray composeTelegram(const QByteArray& head,
                           const QByteArray& data,
                           char terminator = static_cast<char>(0x0A));
