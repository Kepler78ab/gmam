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
#include"Tools.h"
QString OracleDB::m_configPath=NULL;
int OracleDB::m_iConnectionNum = 0;
OracleDB::OracleDB(QObject *parent) : QObject(parent)
{
    // m_ptrOciLib = NULL;


}

OracleDB::~OracleDB()
{

    if(m_timerReConnect)
    {
        m_timerReConnect->stop();
        delete m_timerReConnect;
        m_timerReConnect=nullptr;
    }

    if(m_DB.isOpen())
    {
        // qDebug()<<"oracleDB is open";
        m_DB.close();
        // delete m_DB;
    }
    // m_DB.close();
    // qDebug()<<"析构前连接数"<<m_iConnectionNum;
    m_iConnectionNum--;
    // qDebug()<<"析构后后连接数"<<m_iConnectionNum;
    qDebug()<<"OracleDB close";
   qDebug()<<"OracleDB destroy";
}

// OracleDB *OracleDB::GetInstance()
// {
//     if(NULL == m_instance)
//     {
//         m_instance = new OracleDB;
//     }
//     return m_instance;
// }

void OracleDB::setConfig(const QString& filePath)
{
    m_configPath=filePath;
}

void OracleDB::slotInit()
{
    m_isDBOnline = false;
    m_isNetOnline = false;
    // qDebug()<<"初始化前连接数"<<m_iConnectionNum;
    m_iConnectionNum++;
    // qDebug()<<"初始化后连接数"<<m_iConnectionNum;
    m_strDBConnectionName = "QOCI_Connection"+QString::number(m_iConnectionNum);

    // WriteJson();
    ReadJson();
    Init();
    // Select();
    // m_lastMinDateTime = QDateTime::currentDateTime();

    m_timerReConnect = new QTimer(this);
    connect(m_timerReConnect,SIGNAL(timeout()),this,SLOT(slotReConnectTimeOut()));
    m_timerReConnect->start(m_config.m_reConnectInterval*1000);

}
bool OracleDB::GetDBIsOpen()
{
    return m_DB.isOpen();
}

void OracleDB::Init()
{
    Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("驱动:QDCI%1").arg(QSqlDatabase::isDriverAvailable("QOCI")?"可用":"不可"));
    Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("链接名:QOCI%1").arg((QSqlDatabase::contains("QOCI")?"可用":"不可")));

    m_DB = QSqlDatabase::addDatabase("QOCI",m_strDBConnectionName);
    m_DB.setHostName(m_config.m_strHostName);
    m_DB.setPort(m_config.m_iPort);
    m_DB.setDatabaseName(m_config.m_strDatabaseName);
    m_DB.setUserName(m_config.m_strUserName);
    m_DB.setPassword(m_config.m_strPassword);

    if(!m_DB.open())
    {

        QSqlError err = m_DB.lastError();
        QString str1 = err.databaseText();
        str1 = err.driverText();
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),"数据库打开失败");
        qDebug()<<"--------------------------------";
        qDebug()<<"--------------------HostName:"<<m_DB.hostName();
        qDebug()<<"--------------------Port:"<<m_DB.port();
        qDebug()<<"--------------------DatabaseName:"<<m_DB.databaseName();
        qDebug()<<"--------------------UserName"<<m_DB.userName();
        qDebug()<<"--------------------Password:"<<m_DB.password();
        qDebug()<<"--------------------TableName"<<m_config.m_strTableName;
        qDebug()<<"--------------------------------";
        m_isDBOnline=false;
        m_isNetOnline=false;
        emit dbStatusSignal(false);
        return ;
    }
    m_isDBOnline=true;
    m_isNetOnline=true;
    return ;
}
void OracleDB::ReadJson()
{
    //FILE_NAME_INI
    QString strFileName = m_configPath;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"打开数据库配置文件失败";
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

        m_config.m_moduleName = jsonObject.value("moduleNameDB").toString();

        m_config.m_strHostName = jsonObject.value("IP").toString();

        m_config.m_iPort = jsonObject.value("Port").toInt();

        m_config.m_strDatabaseName = jsonObject.value("DatabaseName").toString();

        m_config.m_strUserName = jsonObject.value("UserName").toString();

        m_config.m_strPassword = jsonObject.value("Password").toString();

        m_config.m_strTableName = jsonObject.value("TableName").toString();


        //PDO config
        m_config.m_serialNo = jsonObject.value("SerialNo").toInt();

        m_config.m_queueId = jsonObject.value("QueueId").toString();

        m_config.m_desc = jsonObject.value("Description").toString();

        m_config.m_reConnectInterval = jsonObject.value("reConnectInterval").toInt();
        // qDebug()<<m_serialNo;
        // qDebug()<<m_queueId;
        // qDebug()<<m_desc;



    }
    // qDebug()<<"m_strHostName"<<m_strHostName;
    // qDebug()<<"m_iPort"<<m_iPort;
    // qDebug()<<"m_strDatabaseName"<<m_strDatabaseName;
    // qDebug()<<"m_strUserName"<<m_strUserName;
    // qDebug()<<"m_strPassword"<<m_strPassword;
    // qDebug()<<"m_strTableName"<<m_strTableName;
}
// void OracleDB::WriteJson()
// {
//     //不写只读
//     QJsonObject object;
//     QJsonArray valueArray1;

//     object.insert("IP",m_strHostName);
//     object.insert("Port",m_iPort);
//     object.insert("DatabaseName",m_strDatabaseName);
//     object.insert("UserName",m_strUserName);
//     object.insert("Password",m_strPassword);
//     object.insert("TableName",m_strTableName);

//     QJsonDocument jsonDoc(object);
//     QString  strFileName = m_configPath;
//     QFile file(strFileName);
//     if(!file.open(QIODevice::WriteOnly))
//     {
//         qDebug()<<"open file:"<<strFileName<<"failed!";
//         return ;
//     }
//     file.write(jsonDoc.toJson());
//     file.close();
// }
int OracleDB::WriteData(const QByteArray& data,const QString& filename)
{
    if(!(ReConnectToNet()&&ReConnectToDB()))
    {
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("网络错误,%1/%2 未写入数据库").arg(QFileInfo(filename).absoluteDir().dirName()).arg(QFileInfo(filename).fileName()));
        emit dbStatusSignal(false);
        return -1;
    }

    // QByteArray *ba = new QByteArray(data);
    QString strData = QString(data);
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
                             .arg(m_config.m_strTableName)
                             .arg(strTime)
                             .arg(m_config.m_serialNo)
                             .arg(m_config.m_queueId)
                             .arg("")
                             .arg(strData)
                             .arg("N")
                             .arg("")
                             .arg("");
    //不写入数据库测试，把这一块注释掉
    //------------------------------------------------------------------
    QSqlQuery query(m_DB);
    query.prepare(strInsert1);
    bool ret = query.exec();//true;// ;
    if(!ret)
    {
        QSqlError err = query.lastError();

        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("Sql Error,%1 未写入数据库").arg(filename));
        qDebug()<<"--------------------------------";
        qDebug()<<"sql exec failed:"<<err.text();
        qDebug()<<"--------------------------------";
        return -1;
    }
    //------------------------------------------------------------------
    // delete ba;
    // ba=nullptr;
    return 0;
}
void OracleDB::slotReConnectTimeOut()
{
    //网络不通情况下，不做数据库连接测试
    //开始就网路不通-》if(!reconnectNet) ->reconnectNet   tmp!=isNetOnline  ->debug(wangluotong)
    bool currentNetStatus=m_isNetOnline;
    bool currentDBStatus=m_isDBOnline;
    ReConnectToNet();
    if(!m_isNetOnline)
    {
        emit dbStatusSignal(false);
        return;
    }
    if(m_isNetOnline!=currentNetStatus&&m_isNetOnline)
    {
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("网络连接：%1 已连通").arg(m_config.m_strHostName));
    }
    if(m_isNetOnline)
    {
        ReConnectToDB();
        if(!m_isDBOnline)
        {
            emit dbStatusSignal(false);
            return;
        }
        if(m_isDBOnline!=currentDBStatus&&m_isDBOnline)
        {
            Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
            emit dbStatusSignal(true);
        }
    }



    //网络是否接通 if(接通）——》数据库是否接通——》是：打印判断m_，否
    // if(ReConnectToNet())
    // {
    //     if(ReConnectToDB())
    //     {
    //         Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("网络连接：%1 已连通").arg(m_config.m_strHostName));
    //         Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
    //         return;
    //     }
    //     else
    //     {
    //         return;
    //     }
    // }


    // qDebug()<<"slot active";
    // // qDebug()<<"内置定时器触发"<<QThread::currentThreadId();
    // if(ReConnectToNet())
    // {
    //     Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("网络连接：%1 已连通").arg(m_config.m_strHostName));
    //     // return;
    //     // return;
    // }
    // else
    // {
    //     Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("网络连接：%1 已连通").arg(m_config.m_strHostName));
    // }
    // if(m_isNetOnline)
    // {
    //     // if(ReConnectToNet())
    //     // {
    //     //     m_bCheckConnectNet = false;

    //     // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString("网络连接：%1已连通").arg(m_strHostName);
    //     if(!ReConnectToDB())
    //     {

    //         // emit dbStatusSignal(true);
    //         // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<;
    //         //数据库重连后，进行数据查询
    //         //slotSelectTimeOut();
    //         return ;
    //     }
    //     else
    //     {
    //         m_isDBOnline = false;
    //         Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
    //     }
    //     // }
    // }
    // else
    // {
    //     //数据库已断开连接
    //     if(m_isDBOnline)
    //     {
    //         //数据库重连
    //         if(ReConnectToDB())
    //         {
    //             m_isDBOnline = false;
    //             Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
    //             // emit dbStatusSignal(true);
    //             // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString("数据库-jydcdb 已连接");
    //             return ;
    //         }
    //         else
    //         {
    //             // emit dbStatusSignal(false);
    //             return;
    //         }
    //     }
    // }
    // bool currentStatus = GetDBIsOpen();
    // emit dbStatusSignal(currentStatus);
    // qDebug()<<"getDbisOpen"<<currentStatus;
    // emit dbStatusSignal(true);
    //数据库已断开连接。。。
}

void OracleDB::slotRecvData(const QByteArray &data,const QString& filename)
{
    if(WriteData(data,filename)==0)
    {
        emit dataWrited(filename);
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString( "写入 %1/%2 到数据库").arg(QFileInfo(filename).absoluteDir().dirName()).arg(QFileInfo(filename).fileName()));
    }
    else
    {
        emit dataNotWrited(filename);
    }


}
bool OracleDB::ReConnectToNet()
{
    //检测IP是否在线
    QProcess cmd;
    QString command = "ping "+m_config.m_strHostName+" -n 1 -w 1000";
    //-n 要发送的回显请求数
    //-w 等待每次回复的超时时间
    cmd.start(command);
    cmd.waitForFinished(1000*1);
    QString retStr = cmd.readAll();
    //CaseInsensitive  大小写不敏感
    if(retStr.indexOf("ttl",0,Qt::CaseInsensitive) == -1)
    {
        //连接不到数据库网
        m_isNetOnline = false;
        m_isDBOnline = false;
        // emit dbStatusSignal(false);

        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("%1 连接失败，等待 %2s 后重连。。。").arg(m_config.m_strHostName).arg(m_config.m_reConnectInterval));
        // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss :")<<m_strHostName+;
        // qDebug()<<"Net offline";
        return false;
    }
    m_isNetOnline = true;
    m_isDBOnline = false;
    // qDebug()<<"Net online";
    // emit dbStatusSignal(true);
    return true;
}
bool OracleDB::ReConnectToDB()
{
    QSqlQuery query(m_DB);
    QString sql = QString::fromLocal8Bit("select 1 from %1").arg(m_config.m_strTableName);

    if(query.exec(sql))
    {
        m_isDBOnline = true;
        emit dbStatusSignal(true);
          // qDebug()<<"DB online";
        return true;
    }
    //数据库重连操作
    m_DB.close();
    m_DB.removeDatabase("QOCI");
    m_DB = QSqlDatabase::addDatabase("QOCI",m_strDBConnectionName);
    m_DB.setHostName(m_config.m_strHostName);
    m_DB.setPort(m_config.m_iPort);
    m_DB.setDatabaseName(m_config.m_strDatabaseName);
    m_DB.setUserName(m_config.m_strUserName);
    m_DB.setPassword(m_config.m_strPassword);
    if(m_DB.open())
    {
        Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库已重新连接"));
        m_isDBOnline = true;
        // emit dbStatusSignal(true);
         // qDebug()<<"DB online";
        return true;
    }
    Tools::log(m_config.m_moduleName,QThread::currentThreadId(),QString("数据库连接失败"));
    m_isDBOnline = false;
    // emit dbStatusSignal(false);
     // qDebug()<<"DB offline";
    return false;
}
