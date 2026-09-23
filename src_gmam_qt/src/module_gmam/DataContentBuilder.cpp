#include "DataContentBuilder.h"
#include "FieldCodec.h"
#include "GmamError.h"

DataContentBuilder::DataContentBuilder(QVector<FieldDetail> schema,
                                       int maxLength)
    : m_schema(std::move(schema))
    , m_maxLength(maxLength)
{
    for (const auto& f : m_schema) {
        if (f.length > 0)
            m_totalLength += f.length;
    }
}

QByteArray DataContentBuilder::build(const QHash<QString, QString>& values,
                                     QStringList& errors) const
{
    errors.clear();

    if (m_totalLength <= 0) {
        errors << QStringLiteral("数据内容总长度为 0");
        return {};
    }
    if (m_totalLength > m_maxLength) {
        errors << QStringLiteral("数据内容超长：%1 > %2")
                    .arg(m_totalLength).arg(m_maxLength);
        return {};
    }

    QByteArray buffer;
    buffer.reserve(m_totalLength);

    for (int i = 0; i < m_schema.size(); ++i) {
        const FieldDetail& f = m_schema.at(i);

        if (f.length <= 0) {
            errors << gmam::formatFieldError(i, f.type, f.name,
                                             QStringLiteral("长度非法"));
            continue;
        }

        if (f.type == FieldType::Reserved) {
            buffer.append(f.length, ' ');
            continue;
        }

        const auto it = values.constFind(f.name);
        const bool keyPresent = (it != values.cend());
        const QString value = keyPresent ? it.value() : QString();

        if (shouldUseMissingFill(f.missingPolicy, keyPresent, value)) {
            buffer.append(gmam::missingPlaceholder(f));
            continue;
        }

        QByteArray fieldBytes;
        QString reason;
        if (!gmam::encodeField(value, f, fieldBytes, reason)) {
            errors << gmam::formatFieldError(i, f.type, f.name, reason);
            buffer.append(gmam::failurePlaceholder(f));
            continue;
        }
        buffer.append(fieldBytes);
    }

    if (!errors.isEmpty())
        return {};
    return buffer;
}
