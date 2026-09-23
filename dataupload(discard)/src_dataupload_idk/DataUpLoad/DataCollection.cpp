#include "DataCollection.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include "OracleDB.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QtMath>

#include <QSettings>
#include <QTextCodec>
#include <QThread>
using namespace std;
#define FILE_STEEL_CONFIG_INI "./SteelConfig.ini"

DataCollection::DataCollection()
{
    /*
    QString strPath = QCoreApplication::applicationDirPath()+"/../File";
    QDir floder(strPath);
    QStringList fileNames = floder.entryList(QDir::Files);
    foreach (QString filename, fileNames)
    {
        //精选卷板线  1开头10位数字项目号 文件是卷板
        if(filename.length()!=14||filename.at(0)!='1')
            continue ;
        cout<<filename.toStdString()<<endl;

        ReadFile(filename);
        cout<<endl;
    }
    */
    //WriteSteelConfig();
    qDebug()<<"DataCollection Object create in "<<QThread::currentThread();
    // static OracleDB ora;
    qDebug()<<"create OracleDb ora";
    ReadSteelConfig();
}

DataCollection::~DataCollection()
{

}

void DataCollection::ReadFile(QString strFileName)
{
    //数据量较大，缺少必要的文件说明，参数暂未获得，待定。
    QString strFullName =  QCoreApplication::applicationDirPath()+"/../File/" + strFileName;
    QFile file(strFullName);
    //打开文件
    if(file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QByteArray fileData = file.readAll();
        //int j =0;
        for(int i =0;i<fileData.length();i++)
        {
            //int-4 float-4 double-8
            //unsigned char d = fileData[i];
            //char data = fileData.at(i);
            unsigned char data = *(unsigned char *)(fileData.data()+i);
            printf(" %d \n",data);
            if(i%100==99)
            {
                getchar();
            }
        }
        file.close();
    }
}
int DataCollection::SendData1(STRU_FILE_DATA fileData)
{
    STRU_T9PHY_TO_ERP sendData;//
    //1-10-传送代号-formId:T9HA:传送拉伸试验实绩到ERP
    memcpy(sendData.m_formId,"T9PHYTOERP",10);
    //2-1-状态-inputCode:N:新增 D:删除
    sendData.m_inputCode = 'N';
    //3-20-批次-lotNo:生产批号
    QByteArray ba = fileData.m_strTestNnumber.toLatin1();
    int ilen = ba.length()>20?20:ba.length();
    //memcpy(sendData.m_lotNo,ba.data(),ilen);
    //4-10-熔炼号****-HeatNo:13805679(炉号)
    //memcpy(sendData.m_HeatNo,"24701771",8);//fileData.m_strTestNnumber
    //5-1-批次试验数量-testNum
    //sendData.m_testNum = '1';
    //6-1-批次试验序号-testSeq
    //sendData.m_testSeq = '1';
    //7-6-班次-shiftWork
    //memset(sendData.m_shiftWork,0,6);
    //8-2-试验状态-sampStatus-----A控轧状态
    //sendData.m_sampStatus[0] = 'A';
    //9-20-试片编号****-sampleId
    QString strTestNum = fileData.m_strTestNnumber + "A";
    ba = strTestNum.toLatin1();
    ilen = ba.length()>20?20:ba.length();
    memcpy(sendData.m_sampleId,ba,ilen);

    //memcpy(sendData.m_sampleId,"1111111120A",11);

    //10-1-试验种类-testType-----常温拉伸
    //sendData.m_testType = '0';
    //11-10-试验状态、方向位置-couponCode-----T尾端C中心部位
    //sendData.m_couponCode[0] = 'T';
    //sendData.m_couponCode[1] = 'C';
    //12-2-本次传送试验项目总共个数-number of analysis elements-----5
    sendData.m_numberOfAnalysisElements[1] = '7';

    //【11-13REPEAT】n为传送元素个数
    //data1-A01屈服强度Re-MPa

    //data1-上屈服强度
    memcpy(sendData.m_data[0].m_element,"A0111",5);
    ba = QString::number(qRound(fileData.m_dA01.toDouble())).toLatin1();//qRound  qFloor
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[0].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[0].m_resultOfElement,"281",3);
    memcpy(sendData.m_data[0].m_unit,"MPa",3);

    //根据批号查询钢种
    //QString strSteelType = GetSteelType(fileData.m_strTestNnumber);
    //根据钢种查询选用上屈服强度A，还是下屈服强度
    //bool bIsHight = GetConfigBySteelType(strSteelType);  A011- 中板数据处理

    //data2-下屈服强度
    memcpy(sendData.m_data[1].m_element,"A0112",5);
    ba = QString::number(qRound(fileData.m_dA011.toDouble())).toLatin1();//qRound qCeil
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[1].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[1].m_resultOfElement,"271",3);
    memcpy(sendData.m_data[1].m_unit,"MPa",3);

    //data3-Rp
    memcpy(sendData.m_data[2].m_element,"A0113",5);
    //ba = fileData.m_dA0113.toLatin1();
    ba = QString::number(qRound(fileData.m_dA0113.toDouble())).toLatin1();
    ilen = ba.length()>10?10:ba.length();
    memcpy(sendData.m_data[2].m_resultOfElement,ba.data(),ilen);
    memcpy(sendData.m_data[2].m_unit,"MPa",3);

    //data4-Rt
    memcpy(sendData.m_data[3].m_element,"A0114",5);
    //ba = fileData.m_dA0114.toLatin1();
    ba = QString::number(qRound(fileData.m_dA0114.toDouble())).toLatin1();
    ilen = ba.length()>10?10:ba.length();
    memcpy(sendData.m_data[3].m_resultOfElement,ba.data(),ilen);
    memcpy(sendData.m_data[3].m_unit,"MPa",3);

    //data5-A02抗拉强度Rm-MPa
    memcpy(sendData.m_data[4].m_element,"A02",3);
    ba = QString::number(qRound(fileData.m_dA02.toDouble())).toLatin1();
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[4].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[4].m_resultOfElement,"352",3);
    memcpy(sendData.m_data[4].m_unit,"MPa",3);

    //data6-A03断后伸长率A-%
    //4.75~5.25=5  5.25~5.75=5.5 5.75~6.25=6
    memcpy(sendData.m_data[5].m_element,"A03",3);
    double dNumber = fileData.m_dA03.toDouble();
    int iNumber = dNumber/1;
    double dCha = dNumber - iNumber;
    if(dCha - 0.25<0.0001)
    {
        dCha = iNumber;
    }
    else if(dCha - 0.75<0.0001)
    {
        dCha = iNumber +0.5;
    }
    else
    {
        dCha = iNumber +1;
    }

    ba = QString::number(dCha,'f',1).toLatin1();//qRound(fileData.m_dA03.toDouble())
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[5].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[5].m_resultOfElement,"46",2);
    memcpy(sendData.m_data[5].m_unit,"%",1);

    //data7-B01--冷弯试验内容
    memcpy(sendData.m_data[6].m_element,"B01",3);
    memcpy(sendData.m_data[6].m_resultOfElement,"Y",1);


    // static OracleDB ora;//*ora = new OracleDB()
    //return OracleDB::GetInstance()->WriteData((char *)&sendData,sizeof(STRU_T9PHY_TO_ERP));
    static OracleDB ora;
    return ora.WriteData((char *)&sendData,sizeof(STRU_T9PHY_TO_ERP));
    // return 1;
}
int DataCollection::SendData(STRU_FILE_DATA fileData)
{
    STRU_T9PHY_TO_ERP sendData;//
    //1-10-传送代号-formId:T9HA:传送拉伸试验实绩到ERP
    memcpy(sendData.m_formId,"T9PHYTOERP",10);
    //2-1-状态-inputCode:N:新增 D:删除
    sendData.m_inputCode = 'N';
    //3-20-批次-lotNo:生产批号
    QByteArray ba = fileData.m_strTestNnumber.toLatin1();
    int ilen = ba.length()>20?20:ba.length();
    //memcpy(sendData.m_lotNo,ba.data(),ilen);
    //4-10-熔炼号****-HeatNo:13805679(炉号)
    //memcpy(sendData.m_HeatNo,"24701771",8);//fileData.m_strTestNnumber
    //5-1-批次试验数量-testNum
    //sendData.m_testNum = '1';
    //6-1-批次试验序号-testSeq
    //sendData.m_testSeq = '1';
    //7-6-班次-shiftWork
    //memset(sendData.m_shiftWork,0,6);
    //8-2-试验状态-sampStatus-----A控轧状态
    //sendData.m_sampStatus[0] = 'A';
    //9-20-试片编号****-sampleId
    QString strTestNum = fileData.m_strTestNnumber + "A";
    ba = strTestNum.toLatin1();
    ilen = ba.length()>20?20:ba.length();
    memcpy(sendData.m_sampleId,ba,ilen);

    //memcpy(sendData.m_sampleId,"1111111120A",11);

    //10-1-试验种类-testType-----常温拉伸
    //sendData.m_testType = '0';
    //11-10-试验状态、方向位置-couponCode-----T尾端C中心部位
    //sendData.m_couponCode[0] = 'T';
    //sendData.m_couponCode[1] = 'C';
    //12-2-本次传送试验项目总共个数-number of analysis elements-----5
    sendData.m_numberOfAnalysisElements[1] = '7';

    //【11-13REPEAT】n为传送元素个数
    //data1-A01屈服强度Re-MPa

    //data1-上屈服强度
    memcpy(sendData.m_data[0].m_element,"A0111",5);
    ba = QString::number(qRound(fileData.m_dA01.toDouble())).toLatin1();//qRound  qFloor
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[0].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[0].m_resultOfElement,"281",3);
    memcpy(sendData.m_data[0].m_unit,"MPa",3);

    //根据批号查询钢种
    //QString strSteelType = GetSteelType(fileData.m_strTestNnumber);
    //根据钢种查询选用上屈服强度A，还是下屈服强度
    //bool bIsHight = GetConfigBySteelType(strSteelType);  A011- 中板数据处理

    //data2-下屈服强度
    memcpy(sendData.m_data[1].m_element,"A0112",5);
    ba = QString::number(qRound(fileData.m_dA011.toDouble())).toLatin1();//qRound qCeil
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[1].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[1].m_resultOfElement,"271",3);
    memcpy(sendData.m_data[1].m_unit,"MPa",3);

    //data3-Rp
    memcpy(sendData.m_data[2].m_element,"A0113",5);
    ba = fileData.m_dA0113.toLatin1();
    ilen = ba.length()>10?10:ba.length();
    memcpy(sendData.m_data[2].m_resultOfElement,ba.data(),ilen);
    memcpy(sendData.m_data[2].m_unit,"MPa",3);

    //data4-Rt
    memcpy(sendData.m_data[3].m_element,"A0114",5);
    ba = fileData.m_dA0114.toLatin1();
    ilen = ba.length()>10?10:ba.length();
    memcpy(sendData.m_data[3].m_resultOfElement,ba.data(),ilen);
    memcpy(sendData.m_data[3].m_unit,"MPa",3);

    //data5-A02抗拉强度Rm-MPa
    memcpy(sendData.m_data[4].m_element,"A02",3);
    ba = QString::number(qRound(fileData.m_dA02.toDouble())).toLatin1();
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[4].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[4].m_resultOfElement,"352",3);
    memcpy(sendData.m_data[4].m_unit,"MPa",3);

    //data6-A03断后伸长率A-%
    //4.75~5.25=5  5.25~5.75=5.5 5.75~6.25=6
    memcpy(sendData.m_data[5].m_element,"A03",3);
    double dNumber = fileData.m_dA03.toDouble();
    int iNumber = dNumber/1;
    double dCha = dNumber - iNumber;
    if(dCha - 0.25<0.0001)
    {
        dCha = iNumber;
    }
    else if(dCha - 0.75<0.0001)
    {
        dCha = iNumber +0.5;
    }
    else
    {
        dCha = iNumber +1;
    }

    ba = QString::number(dCha,'f',1).toLatin1();//qRound(fileData.m_dA03.toDouble())
    ilen = ba.length()>4?4:ba.length();
    memcpy(sendData.m_data[5].m_resultOfElement,ba.data(),ilen);
    //memcpy(sendData.m_data[5].m_resultOfElement,"46",2);
    memcpy(sendData.m_data[5].m_unit,"%",1);

    //data7-B01--冷弯试验内容
    memcpy(sendData.m_data[6].m_element,"B01",3);
    memcpy(sendData.m_data[6].m_resultOfElement,"Y",1);


    // static OracleDB ora;//*ora = new OracleDB()
    //return OracleDB::GetInstance()->WriteData((char *)&sendData,sizeof(STRU_T9PHY_TO_ERP));
    static OracleDB ora;
    return ora.WriteData((char *)&sendData,sizeof(STRU_T9PHY_TO_ERP));
    // return -1;
}

QString DataCollection::GetSteelType(QString strLotNum)
{
    //根据批号获取钢种

    //QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");//,"SQLServer_connection"
    //QString strDSN = QString::fromLocal8Bit("DRIVER={SQL Server};SERVER=10.17.12.251;DATABASE=JYTRKDB;Uid=lhl;Pwd=sa;");
    //db.setDatabaseName(strDSN);
    //bool bIsOpen = db.open();//根据钢种对照配置文件，找上屈服还是下屈服


    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        db = QSqlDatabase::addDatabase("QODBC");//,"QODBC_Connection"
    }
    QString strDSN = QString::fromLocal8Bit("DRIVER={SQL Server};SERVER=10.17.12.251;DATABASE=JYTRKDB;Uid=lhl;Pwd=sa;");

    //表名：HIS_Plan_Data     批号-LOTNUM：2424308 钢种-Steel_grade：Q420B1
    //表名：Plan_Data         批号-LOTNUM：1525535 钢种-Steel_grade：Q235B

    /*

Plan_data
HIS_Plan_data
Record_Plan_data
TRK_AFTER_SIZE_DOWNLINE
HIS_TRK_AFTER_SIZE_DOWNLINE

*/

    db.setDatabaseName(strDSN);
    if(!db.open())
    {
        qDebug()<<"open sql sever database failed:"<<db.lastError().text();
        return "";
    }
    QString strLot = strLotNum.left(strLotNum.size()-2);
    QString strSql = QString("select distinct Steel_grade from Plan_data where lotnum = '%1' ").arg(strLot);
    QSqlQuery query(db);
    bool ret = query.exec(strSql);
    if(ret)
    {
        if(query.next())
        {
            QString strValue = query.value(0).toString();
            return strValue;
        }
        //查到了，没有数据，继续查看HIS_Plan_Data表
        QString strSqlHIS = QString("select DISTINCT Steel_grade from HIS_Plan_data where lotnum = '%1' ").arg(strLot);
        QSqlQuery queryHIS(db);
        bool retHIS = queryHIS.exec(strSqlHIS);
        if(retHIS)
        {
            if(queryHIS.next())
            {
                QString strValue = queryHIS.value(0).toString();
                return strValue;
            }
        }
        else
        {
            QSqlError err = query.lastError();
            qDebug()<<"--------------------SQLServer exec select HIS_Plan_Data failed:"<<err.text();
        }
    }
    else
    {
        QSqlError err = query.lastError();
        qDebug()<<"--------------------SQLServer exec select Plan_Data failed:"<<err.text();
    }
    //查询所有钢种信息
    /*
     *
    QString strSqlHIS = QString("select DISTINCT Steel_grade from HIS_Plan_data");
    QSqlQuery queryHIS(db);
    bool retHIS = queryHIS.exec(strSqlHIS);
    if(retHIS)
    {
        while (queryHIS.next())
        {
            QString strValue = queryHIS.value(0).toString();
            m_mapSteelTypeAndValue.insert(strValue,true);
        }
    }
    else
    {
        QSqlError err = query.lastError();
        qDebug()<<"--------------------SQLServer exec failed:"<<err.text();
    }
    WriteSteelConfig();
    */

    //没有找到，默认上屈服强度
    qDebug()<<"found no Steel Type";
    return "";
}
void DataCollection::ReadSteelConfig()
{
    QString strFileName = QCoreApplication::applicationDirPath()+FILE_STEEL_CONFIG_INI;
    QSettings *settings = new QSettings(strFileName,QSettings::IniFormat);

    //设置文件编码，配置文件中文是，这是必须的，否则乱码
    settings->setIniCodec(QTextCodec::codecForName("UTF8"));

    //按键读取
    //int img_wigth = settings->value("imSize/width").toInt();

    //QString strUser = settings->value("/system/user").toString();

    //QString strLanguage = settings->value("language").toString();

    //按组读取
    settings->beginGroup("SteelType");
    QStringList keys = settings->allKeys();
    QString strKey;
    bool value;
    //初始化Map
    m_mapSteelTypeAndValue.clear();
    for(int i = 0;i<keys.count();i++)
    {
        strKey = keys[i];
        value = settings->value(strKey).toBool();
        m_mapSteelTypeAndValue.insert(strKey,value);
    }
    delete settings;
    return ;
}

void DataCollection::WriteSteelConfig()
{
    QString strFileName = QCoreApplication::applicationDirPath()+FILE_STEEL_CONFIG_INI;
    QSettings *settings = new QSettings(strFileName,QSettings::IniFormat);

    //设置文件编码，配置文件使用中文时，这是必须的，否则乱码；
    settings->setIniCodec(QTextCodec::codecForName("UTF8"));
    settings->clear();

    //直接SetValue(组/键，值)方式写入
   // settings->setValue("/SteelType/SteelType1",11);
    //settings->setValue("/SteelType/SteelType2",12);

    //先设置组，然后setValue(键，值)方式写入，效果同上
    settings->beginGroup("SteelType");
    QMap<QString,bool>::iterator iter =  m_mapSteelTypeAndValue.begin();
    for(;iter != m_mapSteelTypeAndValue.end();iter++)
    {
        settings->setValue(iter.key(),iter.value());
    }
    settings->endGroup();

    //测试写入中文
    //QString strValue = QString::fromLocal8Bit("中文乱码");
    //settings->setValue("Language",strValue);
    delete settings;
    return ;

}

bool DataCollection::GetConfigBySteelType(QString strSteelType)
{
    if(m_mapSteelTypeAndValue.contains(strSteelType))
    {
        return m_mapSteelTypeAndValue.value(strSteelType);
    }
   // qDebug()<<"find no steel type :"<<strSteelType;
    return true;
}




























