#include "XlsxReader.h"
#include <QDebug>
#include <QFile>
#include<QFileInfo>
#include <QThread>
#include<QDateTime>
#include <QFile>
#include<QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "xlsxdocument.h"
#include "Structure.h"
#include "Tools.h"
#include<QCoreApplication>
#include<QFileInfo>
QString ExcelOperator::m_configPath=NULL;
ExcelOperator::ExcelOperator(QObject *parent)
    : QObject{parent}
{
    readJson();
}

ExcelOperator::~ExcelOperator()
{
    // qDebug()<<"ExcelOperator 析构";
}

void ExcelOperator::setThreshold(double minVal, double maxVal)
{
    m_minVal=minVal;
    m_maxVal=maxVal;
}

void ExcelOperator::setConfig(QString configPath)
{
    m_configPath=configPath;
}

void ExcelOperator::readJson()
{
    QString strConfigPath = m_configPath;
    QFile file(strConfigPath);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            m_maxVal = obj.value("maxVal").toDouble();
            m_minVal = obj.value("minVal").toDouble();
            m_unit = obj.value("unit").toString();
            m_formId = obj.value("formId").toString();
        }
    }
    // qDebug()<<"minMax";
    // qDebug()<<m_maxVal<<m_minVal;
}

void ExcelOperator::readFile(QString fileName)
{
    QString d4,d5,k4,k5;
    QStringList rawValues;
    {
        QXlsx::Document xlsx(fileName);
        if (!xlsx.load()) return;

        d4 = xlsx.read("D4").toString().trimmed();
        d5 = xlsx.read("D5").toString().trimmed();
        k4 = xlsx.read("K4").toString().trimmed();
        k5 = xlsx.read("K5").toString().trimmed();

        //收集 20 数据单元格字符串

        auto scanRange = [&](const QString& column, int start, int end) {
            for (int i = start; i <= end; ++i) {
                QVariant val = xlsx.read(column + QString::number(i));
                if (!val.isNull() && !val.toString().trimmed().isEmpty()) {
                    rawValues << val.toString().trimmed();
                }
            }
        };
        scanRange("F", 12, 21);
        scanRange("M", 12, 21);
        QString msg=QString("%1/%2").arg(QFileInfo(fileName).absoluteDir().dirName()).arg(QFileInfo(fileName).fileName());
        Tools::log("XlsxRead",QThread::currentThreadId(),QString("读取文件: %1 结束").arg(msg));
    }
    // QCoreApplication::processEvents();
    STRU_T9PHY_TO_ERP t9;

    //d4 日期
    //k5 设备名
    //d5 试样名称
    //k5 试样批号

    if(!fillT9Data(t9, k5,rawValues))
    {
        // renameXlsxFile(fileName);
        return;
    }
    // qDebug()<<"D4-G4:"<<d4<<" k4-N4:"<<k4;;
    // qDebug()<<"D5-G5:"<<d5<<" K5-N5:"<<k5;
    // qDebug()<<"列数据:"<<rawValues;

    QByteArray t9data;
    t9data.append((char*)&t9, sizeof(STRU_T9PHY_TO_ERP));
    emit sendData(t9data,fileName);
}


void ExcelOperator::copyToFixedBuffer(char *dest, const QString &src, int len)
{
    QByteArray ba = src.toUtf8();
    int actualLen = qMin(ba.length(), len);
    memcpy(dest, ba.data(), actualLen);
}


void ExcelOperator::fillTestData(STRU_TEST_DATA &target, const QString &name, const QString &value, const QString &unit) {
    copyToFixedBuffer(target.m_element, name, 10);
    copyToFixedBuffer(target.m_resultOfElement, value, 10);
    copyToFixedBuffer(target.m_unit, unit, 20);
}

// 封装 T9
bool ExcelOperator::fillT9Data(STRU_T9PHY_TO_ERP &t9,const QString &sampleId,const QStringList &rawValues)
{
    //基础业务字段
    copyToFixedBuffer(t9.m_formId, m_formId, 10);
    t9.m_inputCode = 'N';
    copyToFixedBuffer(t9.m_sampleId, sampleId, 20);
    //testType
    //couponCode
    //20 个数据项的格式化转换


    //数据清洗
    QStringList validValues;
    for (const QString& str : rawValues) {
        bool ok;
        double val = str.toDouble(&ok); // 尝试转为数字

    //"数据超出范围被剔除:" << val << "(要求:" << m_minVal << "-" << m_maxVal << ")"
        if (ok && val >= m_minVal && val <= m_maxVal) {
            validValues << str;
        } else if (ok) {
            // qDebug() << "数据超出范围被剔除:" << val << "(要求:" << m_minVal << "-" << m_maxVal << ")";
        }
    }
    int totalValid = validValues.size();
    if (totalValid < 3) {
        Tools::log("XlsxRead",QThread::currentThreadId(),"符合条件的数据不足 3 个，取消本次报文发送。");
        return false;
    }

    int count = qMin(totalValid, 9);
    QString countStr = QString::number(count).rightJustified(2, ' ');
    copyToFixedBuffer(t9.m_numberOfAnalysisElements, countStr, 2);

    for (int i = 0; i < count; ++i) {
        double dVal = validValues[i].toDouble();
        QString formattedVal = QString::number(dVal, 'f', 3).leftJustified(10, ' ');
        QString elementName = QString("D01%1").arg(i + 1);
        fillTestData(t9.m_data[i], elementName, formattedVal,m_unit);
    }
    return true;
}

