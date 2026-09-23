#pragma once

#include "GmamTypes.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @file HeaderBuilder.h
 * @brief 按 offset 写入定长头部（v0.0.2：字段级错误收集）
 */
class HeaderBuilder {
public:
    explicit HeaderBuilder(QVector<HeaderField> schema,
                           int totalLength,
                           char holeFill = ' ');

    QByteArray build(const QHash<QString, QString>& values,
                     QStringList& errors) const;

    int totalLength() const { return m_totalLength; }

private:
    QVector<HeaderField> m_schema;
    int  m_totalLength = 0;
    char m_holeFill    = ' ';
};
