#include "HeaderBuilder.h"

#include "FieldCodec.h"
#include "GmamError.h"

namespace {

QString headerTypeName(HeaderFieldType t)
{
    switch (t) {
    case HeaderFieldType::Numeric:  return QStringLiteral("Numeric");
    case HeaderFieldType::Chars:    return QStringLiteral("Chars");
    case HeaderFieldType::Reserved: return QStringLiteral("Reserved");
    }
    return QStringLiteral("Unknown");
}

QByteArray headerPlaceholder(const HeaderField& f)
{
    if (f.length <= 0)
        return {};
    const ResolvedFieldStyle style = resolveFieldStyle(f);
    return QByteArray(f.length, style.missingFill);
}

} // namespace

HeaderBuilder::HeaderBuilder(QVector<HeaderField> schema,
                             int totalLength,
                             char holeFill)
    : m_schema(std::move(schema))
    , m_totalLength(totalLength)
    , m_holeFill(holeFill)
{
}

QByteArray HeaderBuilder::build(const QHash<QString, QString>& values,
                                QStringList& errors) const
{
    errors.clear();
    if (m_totalLength <= 0) {
        errors << QStringLiteral("头部总长度非法");
        return {};
    }

    QByteArray buffer(m_totalLength, m_holeFill);

    for (int i = 0; i < m_schema.size(); ++i) {
        const HeaderField& f = m_schema.at(i);

        if (f.offset < 0 || f.length <= 0
            || f.offset + f.length > buffer.size()) {
            errors << gmam::formatFieldError(
                i, headerTypeName(f.type), f.name,
                QStringLiteral("偏移越界（offset=%1 len=%2）")
                    .arg(f.offset).arg(f.length));
            continue;
        }

        QByteArray fieldBytes;
        if (f.type == HeaderFieldType::Reserved) {
            fieldBytes = QByteArray(f.length, ' ');
        } else {
            const auto it = values.constFind(f.name);
            const bool keyPresent = (it != values.cend());
            const QString value = keyPresent ? it.value() : QString();

            if (shouldUseMissingFill(f.missingPolicy, keyPresent, value)) {
                fieldBytes = headerPlaceholder(f);
            } else {
                const FieldDetail tmp = toFieldDetail(f);
                QString reason;
                if (!gmam::encodeField(value, tmp, fieldBytes, reason)) {
                    errors << gmam::formatFieldError(
                        i, headerTypeName(f.type), f.name, reason);
                    fieldBytes = headerPlaceholder(f);
                }
            }
        }

        buffer.replace(f.offset, f.length, fieldBytes);
    }

    if (!errors.isEmpty())
        return {};
    return buffer;
}
