#include "OracleDB.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QProcess>
#include<QThread>
#define FILE_NAME_INI "/./DBJson.ini"
OracleDB *OracleDB::m_instance = NULL;
int OracleDB::m_iConnectionNum = 0;
OracleDB::OracleDB(QObject *parent) : QObject(parent)
{
    m_ptrOciLib = NULL;

    m_bCheckConnectDB = false;
    m_bCheckConnectNet = false;

    m_strHostName = "";
    m_iPort = 0;
    m_strDatabaseName = "";
    m_strUserName = "";
    m_strPassword = "";
    m_strTableName = "";

    m_iConnectionNum++;
    m_strDBConnectionName = "QOCI_Connection"+QString::number(m_iConnectionNum);

    qDebug()<<"";
    qDebug()<<"OracleDB object in Thread"<<QThread::currentThread();
    // WriteJson();
    ReadJson();
    Init();
    // Select();
    m_lastMinDateTime = QDateTime::currentDateTime();

    QTimer *timerReConnect = new QTimer(this);
    connect(timerReConnect,SIGNAL(timeout()),this,SLOT(slotReConnectTimeOut()));
    timerReConnect->start(5000);

}

OracleDB::~OracleDB()
{
    if(NULL != m_ptrOciLib)
    {
        m_ptrOciLib->unload();
        delete m_ptrOciLib;
        m_ptrOciLib = NULL;
    }
    if(m_DB.isOpen())
    {
        qDebug()<<"oracleDB is open";
        m_DB.close();
    }
}

OracleDB *OracleDB::GetInstance()
{
    if(NULL == m_instance)
    {
        m_instance = new OracleDB;
    }
    return m_instance;
}
bool OracleDB::GetDBIsOpen()
{
    return m_DB.isOpen();
}

void OracleDB::Init()
{
    QString strCoreAppPath = QCoreApplication::applicationDirPath();
    m_ptrOciLib = new QLibrary(strCoreAppPath+"/../oracle/oci.dll");
    //qDebug()<<strCoreAppPath;
    m_ptrOciLib->load();
    if(!m_ptrOciLib->isLoaded())
    {
        qDebug()<<"--------------------Load oci.dll failed!";
        return ;
    }
    m_ptrOciLib1 = new QLibrary(strCoreAppPath+"/../oracle/oci.dll");
    //qDebug()<<strCoreAppPath;
    m_ptrOciLib1->load();
    if(!m_ptrOciLib1->isLoaded())
    {
        qDebug()<<"--------------------Load1 oci.dll failed!";
        return ;
    }
    qDebug()<<"QDCI is availible="<<QSqlDatabase::isDriverAvailable("QOCI");
    qDebug()<<"QDCI is availible="<<QSqlDatabase::contains("QOCI");
    m_DB = QSqlDatabase::addDatabase("QOCI",m_strDBConnectionName);
    m_DB.setHostName(m_strHostName);
    m_DB.setPort(m_iPort);
    m_DB.setDatabaseName(m_strDatabaseName);
    m_DB.setUserName(m_strUserName);
    m_DB.setPassword(m_strPassword);

    if(!m_DB.open())
    {
        QSqlError err = m_DB.lastError();
        QString str1 = err.databaseText();
        str1 = err.driverText();
        qDebug()<<"--------------------DB open failed";
        qDebug()<<"--------------------HostName:"<<m_DB.hostName();
        qDebug()<<"--------------------Port:"<<m_DB.port();
        qDebug()<<"--------------------DatabaseName:"<<m_DB.databaseName();
        qDebug()<<"--------------------UserName"<<m_DB.userName();
        qDebug()<<"--------------------Password:"<<m_DB.password();
        qDebug()<<"--------------------TableName"<<m_strTableName;
        return ;
    }



    // qDebug()<<"--------------------DB open success";
    // qDebug()<<"--------------------HostName:"<<m_DB.hostName();
    // qDebug()<<"--------------------Port:"<<m_DB.port();
    // qDebug()<<"--------------------DatabaseName:"<<m_DB.databaseName();
    // qDebug()<<"--------------------UserName"<<m_DB.userName();
    // qDebug()<<"--------------------Password:"<<m_DB.password();
    // qDebug()<<"--------------------TableName"<<m_strTableName;
    //SELECT COLUMN_NAME FROM ALL_TAB_COLUMNS WHERE TABLE_NAME = 'TBDIPDO'
    /*
    TIMESTAMP          //timeStamp         //时戳-20180724023004641
    SERIALNO           //serialNo          //序列号-0
    QUEUEID            //queueID           //队列号-ID61 / HXCHMTOERP
    HEADER             //header            //头？-""
    DATA               //data              //数据--具体数据
    STATUS             //status            //状态-N:尚未处理 0:处理成功 1:DI Client回传失败
    PROCESSTIME        //processTime       //过程时间-20180726 001131
    DESCRIPTION        //description       //描述-“”
    */

    /*
    QString strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    QString strQuery = QString("select * from %1 where queueID = 'T9PHYTOERP' and STATUS='0'").arg(m_strTableName);
    QString strInsert = QString("insert into %1 (TIMESTAMP,SERIALNO,QUEUEID,DATA,STATUS) values('%2','%3','%4','%5','%6')")
            .arg(m_strTableName)
            .arg(strTime)
            .arg(QString("0"))
            .arg("T9PHYTOERP")
            .arg("T9PHYTOERPN                                        C42400729110A                   1K013      34.20    ")
            .arg("N");
    QSqlQuery query(strQuery,m_DB);
    //"T9PHYTOERPN                    20          10 1 1      6  2-C42400729110A       20 1          10 1K013      34.20  "
    //"T9PHYTOERPN                    20          10 1 1      6  2-C42400729110A       20 1          10 1K013      34.20  "
    //"T9PHYTOERPN                    20          10 1 1      6  2-C42400729110A                   1K013      34.20  "

    //"T9PHYTOERPN                                        L32400143110A                   1K021      60.26  "
    //"T9PHYTOERPNX524050095001                           X524053261                     8 A01       447.18    MPa                 A02       617.38    MPa                 A03       24.6      %                   A04       72.43     %
//A07       0         %                   A11       24.5      %                   A12       12.0      %                   B012      4d合格"
    bool ret = query.exec();
    if(!ret)
    {
        QSqlError err = query.lastError();
        qDebug()<<"sql exec failed:"<<err.text();
        return ;
    }
    QStringList strList;
    QSqlRecord sqlRecord(query.record());
    for(int i =0;i<sqlRecord.count();i++)
    {
        strList<<sqlRecord.fieldName(i);

    }
    if(strList.size()<5)return ;

    while(query.next())
    {

       // QString strDate = query.value(0).toString();
       // qDebug()<<strDate;
        QString str = query.value(4).toString();
        if(str.contains("1240776202"))
        {
            for(int i =0;i<strList.size();i++)
            {
                qDebug()<<query.value(strList.at(i));
            }
        }
    }*/
    return ;
}
void OracleDB::ReadJson()
{
    //FILE_NAME_INI
    QString strFileName = QCoreApplication::applicationDirPath() + FILE_NAME_INI;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"open file "<<FILE_NAME_INI<<"failed!";
        return ;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonValue valueArray;

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isObject())
    {
        QJsonObject jsonObject = document.object();
        QJsonValue jsonValue = jsonObject.value("IP");
        if(!jsonValue.isNull())
        {
            m_strHostName = jsonValue.toString();
        }
        jsonValue = jsonObject.value("Port");
        if(!jsonValue.isNull())
        {
            m_iPort = jsonValue.toInt();
        }
        jsonValue = jsonObject.value("DatabaseName");
        if(!jsonValue.isNull())
        {
            m_strDatabaseName = jsonValue.toString();
        }
        jsonValue = jsonObject.value("UserName");
        if(!jsonValue.isNull())
        {
            m_strUserName = jsonValue.toString();
        }
        jsonValue = jsonObject.value("Password");
        if(!jsonValue.isNull())
        {
            m_strPassword = jsonValue.toString();
        }
        jsonValue = jsonObject.value("TableName");
        if(!jsonValue.isNull())
        {
            m_strTableName = jsonValue.toString();
        }
    }
    qDebug()<<"m_strHostName"<<m_strHostName;
    qDebug()<<"m_iPort"<<m_iPort;
    qDebug()<<"m_strDatabaseName"<<m_strDatabaseName;
    qDebug()<<"m_strUserName"<<m_strUserName;
    qDebug()<<"m_strPassword"<<m_strPassword;
    qDebug()<<"m_strTableName"<<m_strTableName;
}
void OracleDB::WriteJson()
{
    //不写只读
    QJsonObject object;
    QJsonArray valueArray1;

    object.insert("IP",m_strHostName);
    object.insert("Port",m_iPort);
    object.insert("DatabaseName",m_strDatabaseName);
    object.insert("UserName",m_strUserName);
    object.insert("Password",m_strPassword);
    object.insert("TableName",m_strTableName);

    QJsonDocument jsonDoc(object);
    QString  strFileName = QCoreApplication::applicationDirPath() + FILE_NAME_INI;
    QFile file(strFileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"open file:"<<strFileName<<"failed!";
        return ;
    }
    file.write(jsonDoc.toJson());
    file.close();
}
int OracleDB::WriteData(char *data,int len)
{
    if((!ReConnectToNet())||(!ReConnectToDB()))
    {
        //数据连接失败
        qDebug()<<"net error";
        return -1;
    }

    QByteArray *ba = new QByteArray(data,len);
    //int alength = ba->length();
    QString strData = QString(*ba);

    /*
    TIMESTAMP          //timeStamp         //时戳-20180724023004641
    SERIALNO           //serialNo          //序列号-0
    QUEUEID            //queueID           //队列号-ID61 / HXCHMTOERP
    HEADER             //header            //头？-""
    DATA               //data              //数据--具体数据
    STATUS             //status            //状态-N:尚未处理 0:处理成功 1:DI Client回传失败
    PROCESSTIME        //processTime       //过程时间-20180726 001131
    DESCRIPTION        //description       //描述-“”
    */

    //确认使用后开启----------------------
    //"20240625151145433"
    //"20240602151145433"
    //20240716151145433
    //20240802151145433
    //20240805151145433
    //20240806151145433
    //20240807151145433
    //20240808151145433
    //20240809151145433
    //20240821151145433
    QString strTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");//QString("20240822151145433");////;
    QString strInsert1 = QString("insert into %1 (TIMESTAMP,SERIALNO,QUEUEID,HEADER,DATA,STATUS,PROCESSTIME,DESCRIPTION) values('%2','%3','%4','%5','%6','%7','%8','%9')")
                             .arg(m_strTableName)
                             .arg(strTime)
                             .arg(40979)
                             .arg("T9PHYTOERP")
                             .arg("")
                             .arg(strData)
                             .arg("N")
                             .arg("")
                             .arg("");
    QSqlQuery query(m_DB);
    query.prepare(strInsert1);
    bool ret = query.exec();//true;// ;
    if(!ret)
    {
        QSqlError err = query.lastError();
        qDebug()<<"sql exec failed:"<<err.text();
        return -1;
    }
    return 0;
}
void OracleDB::Select()
{
    if(!m_DB.isOpen())
    {
        if(!m_DB.open())
        {
            //数据库打不开
            return ;
        }
    }
    QString strTime ="202411051303";// ;

    QString strSelectInfo = QString("select * from %1 where   TIMESTAMP like '%2%' ").arg(m_strTableName).arg(strTime);// .arg("8007") and SERIALNO = %3// and QUEUEID = '%2' and.arg("T9PHYTOERP")
    QSqlQuery query(m_DB);
    query.prepare(strSelectInfo);
    bool ret = query.exec();
    if(!ret)
    {
        QSqlError err = query.lastError();
        qDebug()<<"sql exec failed:"<<err.text();
        return ;
    }
    QStringList strList;
    QSqlRecord sqlRecord(query.record());
    for(int i =0;i<sqlRecord.count();i++)
    {
        strList<<sqlRecord.fieldName(i);

    }
    while(query.next())
    {
        for(int i =0;i<strList.size();i++)
        {
            qDebug()<<strList.at(i)<<"----------"<<query.value(strList.at(i));
        }
        printf("\n");
    }
    return ;
}
void OracleDB::slotReConnectTimeOut()
{
    //网络不通情况下，不做数据库连接测试
    if(m_bCheckConnectNet)
    {
        if(ReConnectToNet())
        {
            m_bCheckConnectNet = false;
            qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString::fromLocal8Bit("网络连接：%1已连通").arg(m_strHostName);
            if(ReConnectToDB())
            {
                m_bCheckConnectDB = false;
                qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString::fromLocal8Bit("数据库-jydcdb 已连接");
                //数据库重连后，进行数据查询
                //slotSelectTimeOut();
                return ;
            }
            else
            {

            }
        }
    }
    else
    {
        //数据库已断开连接
        if(m_bCheckConnectDB)
        {
            //数据库重连
            if(ReConnectToDB())
            {
                m_bCheckConnectDB = false;
                qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString::fromLocal8Bit("数据库-jydcdb 已连接");
                return ;
            }
        }
    }
    //数据库已断开连接。。。
}

bool OracleDB::ReConnectToNet()
{
    //检测IP是否在线
    QProcess cmd;
    QString command = "ping "+m_strHostName+" -n 1 -w 1000";
    //-n 要发送的回显请求数
    //-w 等待每次回复的超时时间
    cmd.start(command);
    cmd.waitForFinished(1000*1);
    QString retStr = cmd.readAll();
    //CaseInsensitive  大小写不敏感
    if(retStr.indexOf("ttl",0,Qt::CaseInsensitive) == -1)
    {
        //连接不到数据库网
        m_bCheckConnectNet = true;
        qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss :")<<m_strHostName+QString::fromLocal8Bit("连接失败，等待重连。。。");
        return false;
    }
    return true;
}
bool OracleDB::ReConnectToDB()
{
    QSqlQuery query(m_DB);
    QString sql = QString::fromLocal8Bit("select 1 from %1").arg(m_strTableName);

    if(query.exec(sql))
    {
        m_bCheckConnectDB = false;
        return  true;
    }
    //数据库重连操作
    m_DB.close();
    m_DB.removeDatabase("QOCI");
    m_DB = QSqlDatabase::addDatabase("QOCI",m_strDBConnectionName);
    m_DB.setHostName(m_strHostName);
    m_DB.setPort(m_iPort);
    m_DB.setDatabaseName(m_strDatabaseName);
    m_DB.setUserName(m_strUserName);
    m_DB.setPassword(m_strPassword);
    if(m_DB.open())
    {
        qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString::fromLocal8Bit("数据库已重新连接")<<"OracleDB::ReConnectToDB";
        m_bCheckConnectDB = false;
        return true;
    }
    qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString::fromLocal8Bit("数据库连接失败")<<"OracleDB::ReConnectToDB";

    m_bCheckConnectDB = true;
    return false;
}
