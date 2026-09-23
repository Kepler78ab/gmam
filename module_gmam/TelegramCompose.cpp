#include "TelegramCompose.h"

QByteArray composeTelegram(const QByteArray& head,
                           const QByteArray& data,
                           char terminator)
{
    QByteArray out;
    out.reserve(head.size() + data.size() + 1);
    out.append(head);
    out.append(data);
    out.append(terminator);
    return out;
}
