#pragma once

#include "GmamTypes.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @file DataContentBuilder.h
 * @brief 按 schema 顺序拼接「数据内容」段（v0.0.2：字段级错误收集）
 *
 * 成功：返回定长字节流，errors 为空。
 * 失败：返回空 QByteArray，errors 列出全部错误（整体级或字段级）。
 */
class DataContentBuilder {
public:
    explicit DataContentBuilder(QVector<FieldDetail> schema,
                                int maxLength = 8162);

    /**
     * @param errors [out] 清空后写入；成功时为空
     */
    QByteArray build(const QHash<QString, QString>& values,
                     QStringList& errors) const;

    int totalLength() const { return m_totalLength; }
    int maxLength() const { return m_maxLength; }

private:
    QVector<FieldDetail> m_schema;
    int m_totalLength = 0;
    int m_maxLength   = 8162;
};
