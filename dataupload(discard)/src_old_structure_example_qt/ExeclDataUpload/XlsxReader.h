#ifndef EXCELOPERATOR_H
#define EXCELOPERATOR_H

#include <QObject>
#include<QString>
#include "Structure.h"
class ExcelOperator : public QObject
{
    Q_OBJECT
public:
    explicit ExcelOperator(QObject *parent = nullptr);
    ~ExcelOperator();
    void setThreshold(double minVal,double maxVal);
    static void setConfig(QString configPath);//用
private:
    void readJson();
    void copyToFixedBuffer(char *dest,const QString&src,int len);
    void fillTestData(STRU_TEST_DATA &target, const QString &name, const QString &value, const QString &unit);
    bool fillT9Data(STRU_T9PHY_TO_ERP &t9,const QString &sampleId,const QStringList &rawValues);
signals:
    void sendData(QByteArray data,QString filename);
public slots:
    void readFile(QString fileName);
private:
    static QString m_configPath;
    double m_minVal;
    double m_maxVal;
    QString m_formId;
    QString m_unit;
};

#endif // EXCELOPERATOR_H
