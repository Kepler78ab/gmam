#ifndef GMAM_HEADERBUILDER_H
#define GMAM_HEADERBUILDER_H

#include "HeaderTypes.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @file HeaderBuilder.h
 * @brief 按 offset 写入定长头部（已移出 module_gmam）
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

#endif // GMAM_HEADERBUILDER_H
