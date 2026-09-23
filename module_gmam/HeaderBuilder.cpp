#include "HeaderBuilder.h"
#include "FieldCodec.h"
#include "GmamError.h"

namespace {

FieldDetail toFieldDetail(const HeaderField& f)
{
    FieldDetail d;
    d.name          = f.name;
    d.type          = toFieldType(f.type);
    d.length        = f.length;
    d.scale         = f.scale;
    d.rounding      = f.rounding;
    d.overflow      = f.overflow;
    d.overflowFill  = f.overflowFill;
    d.align         = f.align;
    d.padFill       = f.padFill;
    d.missingFill   = f.missingFill;
    d.missingPolicy = f.missingPolicy;
    return d;
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
                i, f.type, f.name,
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
                fieldBytes = gmam::missingPlaceholder(f);
            } else {
                const FieldDetail tmp = toFieldDetail(f);
                QString reason;
                if (!gmam::encodeField(value, tmp, fieldBytes, reason)) {
                    errors << gmam::formatFieldError(i, f.type, f.name, reason);
                    fieldBytes = gmam::failurePlaceholder(f);
                }
            }
        }

        buffer.replace(f.offset, f.length, fieldBytes);
    }

    if (!errors.isEmpty())
        return {};
    return buffer;
}
