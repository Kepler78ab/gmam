#pragma once

#include "GmamTypes.h"

#include <QByteArray>
#include <QString>

namespace gmam {

inline QString fieldTypeName(FieldType t)
{
    switch (t) {
    case FieldType::Numeric:  return QStringLiteral("Numeric");
    case FieldType::Chars:    return QStringLiteral("Chars");
    case FieldType::Reserved: return QStringLiteral("Reserved");
    }
    return QStringLiteral("Unknown");
}

inline QString fieldTypeName(HeaderFieldType t)
{
    switch (t) {
    case HeaderFieldType::Numeric:  return QStringLiteral("Numeric");
    case HeaderFieldType::Chars:    return QStringLiteral("Chars");
    case HeaderFieldType::Reserved: return QStringLiteral("Reserved");
    }
    return QStringLiteral("Unknown");
}

inline QString formatFieldError(int index,
                                const QString& typeName,
                                const QString& name,
                                const QString& reason)
{
    return QStringLiteral("字段 %1（第 %2 个，%3）：%4")
        .arg(name)
        .arg(index)
        .arg(typeName)
        .arg(reason);
}

inline QString formatFieldError(int index, FieldType type,
                                const QString& name, const QString& reason)
{
    return formatFieldError(index, fieldTypeName(type), name, reason);
}

inline QString formatFieldError(int index, HeaderFieldType type,
                                const QString& name, const QString& reason)
{
    return formatFieldError(index, fieldTypeName(type), name, reason);
}

/** 失败占位：使用 resolve 后的 missingFill（v0.0.3） */
inline QByteArray failurePlaceholder(const FieldDetail& f)
{
    if (f.length <= 0)
        return {};
    const ResolvedFieldStyle style = resolveFieldStyle(f);
    return QByteArray(f.length, style.missingFill);
}

inline QByteArray failurePlaceholder(const HeaderField& f)
{
    if (f.length <= 0)
        return {};
    const ResolvedFieldStyle style = resolveFieldStyle(f);
    return QByteArray(f.length, style.missingFill);
}

inline QByteArray missingPlaceholder(const FieldDetail& f)
{
    return failurePlaceholder(f);
}

inline QByteArray missingPlaceholder(const HeaderField& f)
{
    return failurePlaceholder(f);
}

} // namespace gmam
