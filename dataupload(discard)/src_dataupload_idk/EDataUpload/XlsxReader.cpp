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

#include "xls.h"
using namespace xls;

ExcelOperator::ExcelOperator(const XLSXR_CONFIG& config,QObject *parent)
    : QObject{parent}
    ,m_config(config)
{
    // qDebug()<<m_config.m_minVal<<""<<m_config.m_maxVal;
    // qDebug()<<m_config.m_useCustomInterval;
}

ExcelOperator::~ExcelOperator()
{
    qDebug()<<"ExcelOperator destroy";
}

void ExcelOperator::processExecl(const QString &filename,int type)
{
    if(type!=0)
        return;
    QFileInfo fileInfo(filename);
    QString suffix = fileInfo.suffix().toLower();
    XLSX_DATA data;
    if (suffix == "xlsx") {
        // 调用现有的方法
        readXlsx(filename,data);
    }
    else if (suffix == "xls") {
        // 调用新写的旧版读取方法
        readXls(filename,data);
    }
    else {
        Tools::log("Error", QThread::currentThreadId(), "不支持的文件格式: " + suffix);
    }
    if(m_config.m_isDebugXlsData)
    {
        QTextStream(stdout)<<"--------"<<fileInfo.fileName()<<"--------\n"
                            <<" DateTime:"<<data.m_date<<'\n'
                            <<" SampleId:"<<data.m_sampleName<<'\n'
                            <<" equipName:"<<data.m_equipName<<'\n'
                            <<" batchNum:"<<data.m_batchNum<<'\n'
                            <<" testLoad:"<<data.m_testLoad<<'\n'
                            <<" Unit:"<<data.m_unit<<'\n';

        qDebug()<<"data:"<<data.m_rawValues<<"\n------------------------------";
    }

    STRU_T9PHY_TO_ERP t9;

    //d4 日期
    //k5 设备名
    //d5 试样名称
    //k5 试样批号

    if(!fillT9Data(t9, data))
    {
        return;
    }
    QByteArray t9data;
    t9data.append((char*)&t9, sizeof(STRU_T9PHY_TO_ERP));
    emit sendData(t9data,filename);
}

QPoint ExcelOperator::excelToPos(const QString &ref)
{
    if (ref.isEmpty()) return QPoint(-1, -1);
    int col = ref.at(0).toUpper().unicode() - 'A';
    int row = ref.mid(1).toInt() - 1;
    return QPoint(col, row);

}
void ExcelOperator::readXlsx(const QString &fileName,XLSX_DATA &data)
{

    QXlsx::Document xlsx(fileName);
    if (!xlsx.load()) return;

    data.m_date = xlsx.read(m_config.m_location_date).toString().trimmed();
    data.m_sampleName = xlsx.read(m_config.m_location_sampleName).toString().trimmed();
    data.m_equipName = xlsx.read(m_config.m_location_equipName).toString().trimmed();
    data.m_batchNum = xlsx.read(m_config.m_location_batchNum).toString().trimmed();
    data.m_testLoad = xlsx.read(m_config.m_location_testLoad).toString().trimmed();
    data.m_unit=xlsx.read(m_config.m_location_unit).toString().trimmed();
    // QString m_location_date="D4";
    // QString m_location_equipName="K4";
    // QString m_location_sampleName="D5";
    // QString m_location_batchNum="K5";
    // QString m_location_testLoad="D8";
    // QString m_location_unit="F11";





    // data.m_maxVal=xlsx.read("A9").toString().trimmed();

    QVariant valUnit = xlsx.read(m_config.m_location_unit);
    if (!valUnit.isNull() && !valUnit.toString().trimmed().isEmpty()) {
        data.m_unit =valUnit.toString().trimmed();
    }


    //收集 20 数据单元格字符串

    auto scanRange = [&](const QString&loc) {
        QStringList params=loc.split(',');
        QString column=params[0];
        int start=params[1].toInt();
        int end=params[2].toInt();
        // const QString& column, int start, int end
        for (int i = start; i <= end; ++i) {
            QVariant val = xlsx.read(column + QString::number(i));
            if (!val.isNull() && !val.toString().trimmed().isEmpty()) {
                data.m_rawValues << val.toString().trimmed();
            }
        }
    };

    scanRange(m_config.m_loaction_rawValue1);
    scanRange(m_config.m_loaction_rawValue2);

    QFileInfo tmp(fileName);
    Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("读取文件: %1/%2 结束").arg(tmp.absoluteDir().dirName()).arg(tmp.fileName()));

    // return data;

}

void ExcelOperator::readXls(const QString &fileName,XLSX_DATA &data)
{
    // 1. 打开工作簿 (使用 UTF-8 确保中文不乱码)
    xlsWorkBook* pWB = xls_open(fileName.toLocal8Bit().data(), "UTF-8");
    if (!pWB) {
        Tools::log("Error", QThread::currentThreadId(), "无法打开旧版文件: " + fileName);
        return ;
    }

    // 2. 获取第一个工作表
    xlsWorkSheet* pWS = xls_getWorkSheet(pWB, 0);
    if (!pWS) {
        xls_close_WB(pWB);
        return ;
    }
    xls_parseWorkSheet(pWS);

    // 辅助 Lambda：读取指定坐标内容
    auto readCell = [&](const QString& ref) -> QString {
        QPoint pos = excelToPos(ref);

        xlsCell* cell = xls_cell(pWS, pos.y(), pos.x());
        if (cell && cell->str) {
            // qDebug()<<pos<<" return Not void ";
            return QString::fromUtf8((char*)cell->str).trimmed();
        }
        // qDebug()<<pos<<" return void";
        return QString();
    };


    // QString m_location_date="D4";
    // QString m_location_equipName="K4";
    // QString m_location_sampleName="D5";
    // QString m_location_batchNum="K5";
    // QString m_location_testLoad="D8";
    // QString m_location_unit="F11";
    // qDebug()


    // qDebug()<<readCell(m_config.m_location_date);
    // qDebug()<<readCell(m_config.m_location_sampleName);
    // qDebug()<<readCell(m_config.m_location_equipName);
    // qDebug()<<readCell(m_config.m_location_batchNum);
    // qDebug()<<readCell(m_config.m_location_testLoad);
    // qDebug()<<readCell(m_config.m_location_unit);

    data.m_date = readCell(m_config.m_location_date);
    data.m_sampleName = readCell(m_config.m_location_sampleName);
    data.m_equipName = readCell(m_config.m_location_equipName);
    data.m_batchNum = readCell(m_config.m_location_batchNum);
    data.m_testLoad = readCell(m_config.m_location_testLoad);
    data.m_unit = readCell(m_config.m_location_unit);

    // 4. 扫描范围数据 (F12-21, M12-21)
    auto scanRangeOld = [&](const QString&loc) {
        QStringList params=loc.split(',');
        QString column=params[0];
        int start=params[1].toInt();
        int end=params[2].toInt();
        for (int i = start; i <= end; ++i) {

            QString val = readCell(column + QString::number(i));
            if (!val.isEmpty()) {
                data.m_rawValues << val;
                // qDebug()<<val;
            }
        }
    };
    scanRangeOld(m_config.m_loaction_rawValue1);
    scanRangeOld(m_config.m_loaction_rawValue2);

    // 5. 日志输出
    QFileInfo tmp(fileName);
    Tools::log(m_config.m_moduleName, QThread::currentThreadId(),
               QString("读取旧版文件: %1/%2 结束").arg(tmp.absoluteDir().dirName()).arg(tmp.fileName()));

    // 6. 释放内存
    xls_close_WS(pWS);
    xls_close_WB(pWB);

    // QTextStream(stdout)<<"--------"<<"fileInfo.fileName()"<<"--------\n"
    //                     <<" DateTime:"<<data.m_date<<'\n'
    //                     <<" SampleId:"<<data.m_sampleName<<'\n'
    //                     <<" equipName:"<<data.m_equipName<<'\n'
    //                     <<" batchNum:"<<data.m_batchNum<<'\n'
    //                     <<" testLoad:"<<data.m_testLoad<<'\n'
    //                     <<" Unit:"<<data.m_unit<<'\n';

    // qDebug()<<"data:"<<data.m_rawValues<<"\n------------------------------";
    // return data;
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
bool ExcelOperator::fillT9Data(STRU_T9PHY_TO_ERP &t9,const XLSX_DATA &data)
{
    //基础业务字段
    copyToFixedBuffer(t9.m_formId, m_config.m_formId, 10);
    t9.m_inputCode = 'N';
    copyToFixedBuffer(t9.m_sampleId, data.m_batchNum, 20);
    //testType
    //couponCode
    //20 个数据项的格式化转换


    //数据清洗
    QStringList validValues;
    double minVal,maxVal;
    QString unit;
    if(!m_config.m_useCustomInterval)
    {
        minVal=m_config.m_minVal;
        maxVal=m_config.m_maxVal;
    }
    else
    {

    }
    if(!m_config.m_useCustomUnit)
    {
        unit=m_config.m_unit;
    }
    else
    {
        unit=data.m_unit;
    }
    for (const QString& str : data.m_rawValues) {
        bool ok;
        double val = str.toDouble(&ok); // 尝试转为数字


        if (ok && val >= minVal && val <= maxVal) {
            validValues << str;
        } else if (ok) {
            // qDebug()<<"数据超出范围被剔除:" << val << "(要求:" << minVal << "-" << maxVal << ")";
            // qDebug() << "数据超出范围被剔除:" << val << "(要求:" << m_minVal << "-" << m_maxVal << ")";
        }
    }
    int totalValid = validValues.size();
    if (totalValid < 3) {
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),"符合条件的数据不足 3 个，取消本次报文发送。");
        return false;
    }

    int count = qMin(totalValid, 9);
    QString countStr = QString::number(count).rightJustified(2, ' ');
    copyToFixedBuffer(t9.m_numberOfAnalysisElements, countStr, 2);

    for (int i = 0; i < count; ++i) {
        double dVal = validValues[i].toDouble();
        QString formattedVal = QString::number(dVal, 'f', 3).leftJustified(10, ' ');
        QString elementName = QString("D01%1").arg(i + 1);
        fillTestData(t9.m_data[i], elementName, formattedVal,unit);
    }
    return true;
}

