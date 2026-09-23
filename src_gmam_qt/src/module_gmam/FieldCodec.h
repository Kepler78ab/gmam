#pragma once

#include "GmamTypes.h"

#include <QByteArray>
#include <QString>

namespace gmam {

bool parseNumericString(const QString& input,
                        int scale,
                        Rounding rounding,
                        QString& digitsStr,
                        QString& reason);

/**
 * 按 FieldDetail 编码；对齐/补位/超额截断使用 resolveFieldStyle。
 * @param reason 失败短句
 */
bool encodeField(const QString& value,
                 const FieldDetail& f,
                 QByteArray& out,
                 QString& reason);

} // namespace gmam
