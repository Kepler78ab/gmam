#ifndef EXCELOPERATOR_H
#define EXCELOPERATOR_H

#include <QObject>
#include<QString>
#include "Structure.h"

struct XLSXR_CONFIG
{
    QString m_moduleName="XLSXREAD";
    double m_minVal;
    double m_maxVal;
    QString m_formId;
    QString m_unit;
    bool m_useCustomInterval=false;
    bool m_useCustomUnit=false;

    QString m_location_date="D4";
    QString m_location_equipName="K4";
    QString m_location_sampleName="D5";
    QString m_location_batchNum="K5";
    QString m_location_testLoad="D8";
    QString m_location_unit="F11";
    QString m_loaction_rawValue1="F,12,21";
    QString m_loaction_rawValue2="M,12,21";

    bool m_isDebugXlsData=false;


    XLSXR_CONFIG() {}
};
struct XLSX_DATA
{
    QString m_date=NULL;
    QString m_sampleName=NULL;
    QString m_batchNum=NULL;
    QString m_testLoad=NULL;
    QString m_unit="HV";
    QString m_equipName=NULL;
    QStringList m_rawValues;
    double m_minVal,m_maxVal;
    XLSX_DATA(){};

};

class ExcelOperator : public QObject
{
    Q_OBJECT
public:
    explicit ExcelOperator(const XLSXR_CONFIG& config,QObject *parent = nullptr);
   virtual ~ExcelOperator();
public slots:
    void processExecl(const QString&filename,int type);
private:
    QPoint excelToPos(const QString& ref);
    void copyToFixedBuffer(char *dest,const QString&src,int len);
    void fillTestData(STRU_TEST_DATA &target, const QString &name, const QString &value, const QString &unit);
    bool fillT9Data(STRU_T9PHY_TO_ERP &t9,const XLSX_DATA &data);
    // void readFile(const QString& fileName);
    // void readOldFile(const QString& fileName);
    void readXlsx(const QString& fileName,XLSX_DATA &data);
    void readXls(const QString& fileName,XLSX_DATA &data);
signals:
    void sendData(const QByteArray& data,const QString &filename);
private:
    XLSXR_CONFIG m_config;

};

#endif // EXCELOPERATOR_H
