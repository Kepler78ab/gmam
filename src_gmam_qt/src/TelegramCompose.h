#ifndef GMAM_TELEGRAMCOMPOSE_H
#define GMAM_TELEGRAMCOMPOSE_H

#include <QByteArray>

/**
 * @file TelegramCompose.h
 * @brief 外层简单拼装：head + data + 结束符（已移出 module_gmam）
 */
QByteArray composeTelegram(const QByteArray& head,
                           const QByteArray& data,
                           char terminator = static_cast<char>(0x0A));

#endif // GMAM_TELEGRAMCOMPOSE_H
