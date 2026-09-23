#include "FieldCodec.h"

namespace gmam {
namespace {

QByteArray padToLength(const QByteArray& raw, int length,
                       FieldAlign align, char padFill)
{
    if (raw.size() >= length)
        return raw;
    const int n = length - raw.size();
    if (align == FieldAlign::Left)
        return raw + QByteArray(n, padFill);
    return QByteArray(n, padFill) + raw;
}

QByteArray truncateToLength(const QByteArray& raw, int length, FieldAlign align)
{
    if (raw.size() <= length)
        return raw;
    if (align == FieldAlign::Left)
        return raw.left(length);
    return raw.right(length);
}

} // namespace

bool parseNumericString(const QString& input,
                        int scale,
                        Rounding rounding,
                        QString& digitsStr,
                        QString& reason)
{
    digitsStr.clear();

    if (scale < 0) {
        reason = QStringLiteral("scale 非法");
        return false;
    }

    QString s = input.trimmed();
    if (s.isEmpty()) {
        reason = QStringLiteral("数值为空");
        return false;
    }
    if (s.startsWith(QLatin1Char('-'))) {
        reason = QStringLiteral("不支持负数");
        return false;
    }
    if (s.startsWith(QLatin1Char('+')))
        s.remove(0, 1);

    const int dotPos = s.indexOf(QLatin1Char('.'));
    QString intPart  = (dotPos < 0) ? s : s.left(dotPos);
    QString fracPart = (dotPos < 0) ? QString() : s.mid(dotPos + 1);
    if (intPart.isEmpty())
        intPart = QStringLiteral("0");

    for (const QChar c : intPart) {
        if (!c.isDigit()) {
            reason = QStringLiteral("含非法字符");
            return false;
        }
    }
    for (const QChar c : fracPart) {
        if (!c.isDigit()) {
            reason = QStringLiteral("含非法字符");
            return false;
        }
    }

    QString scaledFrac;
    if (fracPart.size() >= scale) {
        scaledFrac = fracPart.left(scale);
        if (rounding == Rounding::Round
            && scale < fracPart.size()
            && fracPart.at(scale) >= QLatin1Char('5')) {
            QString whole = intPart + scaledFrac;
            int i = whole.size() - 1;
            while (i >= 0) {
                if (whole.at(i) < QLatin1Char('9')) {
                    whole[i] = QChar(whole.at(i).unicode() + 1);
                    break;
                }
                whole[i] = QLatin1Char('0');
                --i;
            }
            if (i < 0)
                whole.prepend(QLatin1Char('1'));
            digitsStr = whole;
            return true;
        }
    } else {
        scaledFrac = fracPart;
        scaledFrac.append(QString(scale - fracPart.size(), QLatin1Char('0')));
    }

    digitsStr = intPart + scaledFrac;
    while (digitsStr.size() > 1 && digitsStr.at(0) == QLatin1Char('0'))
        digitsStr.remove(0, 1);
    return true;
}

bool encodeField(const QString& value,
                 const FieldDetail& f,
                 QByteArray& out,
                 QString& reason)
{
    out.clear();
    if (f.length <= 0) {
        reason = QStringLiteral("长度非法");
        return false;
    }

    const ResolvedFieldStyle style = resolveFieldStyle(f);

    switch (f.type) {
    case FieldType::Numeric: {
        if (f.length > 18) {
            reason = QStringLiteral("数值长度超限（%1 > 18）").arg(f.length);
            return false;
        }
        QString digitsStr;
        if (!parseNumericString(value, f.scale, f.rounding, digitsStr, reason))
            return false;
        QByteArray raw = digitsStr.toLatin1();
        if (raw.size() > f.length) {
            switch (f.overflow) {
            case OverflowPolicy::FillChar:
                out = QByteArray(f.length, f.overflowFill);
                return true;
            case OverflowPolicy::Truncate:
                raw = truncateToLength(raw, f.length, style.align);
                break;
            case OverflowPolicy::Error:
                reason = QStringLiteral("数值溢出（%1 > %2）")
                             .arg(raw.size()).arg(f.length);
                return false;
            }
        }
        out = padToLength(raw, f.length, style.align, style.padFill);
        return true;
    }
    case FieldType::Chars: {
        QByteArray raw = value.toLatin1();
        if (raw.size() > f.length) {
            switch (f.overflow) {
            case OverflowPolicy::FillChar:
                out = QByteArray(f.length, f.overflowFill);
                return true;
            case OverflowPolicy::Truncate:
                raw = truncateToLength(raw, f.length, style.align);
                break;
            case OverflowPolicy::Error:
                reason = QStringLiteral("字符超长（%1 > %2）")
                             .arg(raw.size()).arg(f.length);
                return false;
            }
        }
        out = padToLength(raw, f.length, style.align, style.padFill);
        return true;
    }
    case FieldType::Reserved: {
        out = QByteArray(f.length, ' ');
        return true;
    }
    }
    reason = QStringLiteral("未知字段类型");
    return false;
}

} // namespace gmam
