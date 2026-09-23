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
OracleDB *OracleDB::m_instance = NULL;
QString OracleDB::m_configPath=NULL;
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

    // WriteJson();
    ReadJson();
    Init();
    // Select();
    m_lastMinDateTime = QDateTime::currentDateTime();

    m_timerReConnect = new QTimer(this);
    connect(m_timerReConnect,SIGNAL(timeout()),this,SLOT(slotReConnectTimeOut()));
    m_timerReConnect->start(5000);

}

OracleDB::~OracleDB()
{
    qDebug()<<"OracleDB 析构";
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

void OracleDB::setConfig(const QString& filePath)
{
    m_configPath=filePath;
}
bool OracleDB::GetDBIsOpen()
{
    return m_DB.isOpen();
}

void OracleDB::Init()
{
    // QString strCoreAppPath = QCoreApplication::applicationDirPath();
    // m_ptrOciLib = new QLibrary(strCoreAppPath+"/../oracle/oci.dll");
    // //qDebug()<<strCoreAppPath;
    // m_ptrOciLib->load();
    // if(!m_ptrOciLib->isLoaded())
    // {
    //     qDebug()<<"--------------------Load oci.dll failed!";
    //     return ;
    // }
    // m_ptrOciLib1 = new QLibrary(strCoreAppPath+"/../oracle/oci.dll");
    // //qDebug()<<strCoreAppPath;
    // m_ptrOciLib1->load();
    // if(!m_ptrOciLib1->isLoaded())
    // {
    //     qDebug()<<"--------------------Load1 oci.dll failed!";
    //     return ;
    // }
    Tools::log("OracleDB",QThread::currentThreadId(),QString("驱动:QDCI%1").arg(QSqlDatabase::isDriverAvailable("QOCI")?"可用":"不可"));
    Tools::log("OracleDB",QThread::currentThreadId(),QString("链接名:QOCI%1").arg((QSqlDatabase::contains("QOCI")?"可用":"不可")));

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
        Tools::log("OracleDB",QThread::currentThreadId(),"数据库打开失败");
        qDebug()<<"--------------------------------";
        qDebug()<<"--------------------HostName:"<<m_DB.hostName();
        qDebug()<<"--------------------Port:"<<m_DB.port();
        qDebug()<<"--------------------DatabaseName:"<<m_DB.databaseName();
        qDebug()<<"--------------------UserName"<<m_DB.userName();
        qDebug()<<"--------------------Password:"<<m_DB.password();
        qDebug()<<"--------------------TableName"<<m_strTableName;
        qDebug()<<"--------------------------------";
        return ;
    }
    // emit dbStatusSignal(m_DB.open());


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

        //PDO config
        m_serialNo = jsonObject.value("SerialNo").toInt();

        m_queueId = jsonObject.value("QueueId").toString();

        m_desc = jsonObject.value("Description").toString();
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
    QString  strFileName = m_configPath;
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
                             .arg(m_serialNo)
                             .arg(m_queueId)
                             .arg("")
                             .arg(strData)
                             .arg("N")
                             .arg("")
                             .arg(m_desc);
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

int OracleDB::WriteData(QByteArray data,const QString& filename)
{
    if((!ReConnectToNet())||(!ReConnectToDB()))
    {
        //数据连接失败
        // qDebug()<<"net error";
        Tools::log("OracleDb",QThread::currentThreadId(),QString("Net Error,%1 Not write to DB").arg(filename));
        // emit dataNotWrited(filename);
        return -1;
    }

    QByteArray *ba = new QByteArray(data);
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
                             .arg(m_serialNo)
                             .arg(m_queueId)
                             .arg("")
                             .arg(strData)
                             .arg("N")
                             .arg("")
                             .arg(m_desc);
    //不写入数据库测试，把这一块注释掉
    //------------------------------------------------------------------
    QSqlQuery query(m_DB);
    query.prepare(strInsert1);
    bool ret = query.exec();//true;// ;
    if(!ret)
    {
        QSqlError err = query.lastError();

        Tools::log("OracleDb",QThread::currentThreadId(),QString("Sql Error,%1 Not write to DB").arg(filename));
        qDebug()<<"--------------------------------";
        qDebug()<<"sql exec failed:"<<err.text();
        qDebug()<<"--------------------------------";
        return -1;
    }
    //------------------------------------------------------------------
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

    // qDebug()<<"内置定时器触发"<<QThread::currentThreadId();
    if(!ReConnectToNet())
    {
        emit dbStatusSignal(false);
    }
    if(m_bCheckConnectNet)
    {
        if(ReConnectToNet())
        {
            m_bCheckConnectNet = false;
            Tools::log("OracleDB",QThread::currentThreadId(),QString("网络连接：%1已连通").arg(m_strHostName));
            // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString("网络连接：%1已连通").arg(m_strHostName);
            if(ReConnectToDB())
            {
                m_bCheckConnectDB = false;
                Tools::log("OracleDB",QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
                emit dbStatusSignal(true);
                // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<;
                //数据库重连后，进行数据查询
                //slotSelectTimeOut();
                return ;
            }
            else
            {
                emit dbStatusSignal(false); // 【关键点2：网络通但数据库没通】
                return;
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
                Tools::log("OracleDB",QThread::currentThreadId(),QString("数据库-jydcdb 已连接"));
                emit dbStatusSignal(true);
                // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString("数据库-jydcdb 已连接");
                return ;
            }
            else
            {
                emit dbStatusSignal(false);
                return;
            }
        }
    }
    // bool currentStatus = GetDBIsOpen();
    // emit dbStatusSignal(currentStatus);
    // qDebug()<<"getDbisOpen"<<currentStatus;
    // emit dbStatusSignal(true);
    //数据库已断开连接。。。
}

void OracleDB::slotRecvData(QByteArray data,const QString& filename)
{
    if( WriteData(data,filename)==0)
    {
        emit dataWrited(filename);
        Tools::log("OracleDB",QThread::currentThreadId(),QString( "写入 %1/%2 到数据库").arg(QFileInfo(filename).absoluteDir().dirName()).arg(QFileInfo(filename).fileName()));
    }
    else
    {
        emit dataNotWrited(filename);
    }


}

void OracleDB::moveToThread(QThread *targetThread)
{
    QObject::moveToThread(targetThread);

    // // 停止并删除旧的定时器
    // if(timerReConnect) {
    //     timerReConnect->stop();
    //     timerReConnect->deleteLater();
    //     timerReConnect = nullptr;
    // }

    // 在新的线程中创建新的定时器
    // QTimer *timerReConnect = new QTimer(this);
    // connect(timerReConnect, SIGNAL(timeout()), this, SLOT(slotReConnectTimeOut()));
    // connect(this->thread(), &QThread::started, [=]() {
    //     timerReConnect->start(5000);
    // });
}

void OracleDB::startReconnectTimer()
{
    // qDebug()<<"slot-startReconnectTimer"<<QThread::currentThreadId();
    /*if(this->thread() == QThread::currentThread()) {
        m_timerReConnect->start(5000);
        qDebug() << "重连定时器在目标线程启动，线程ID：" << QThread::currentThreadId();
    } else {
        qWarning() << "尝试在错误线程启动定时器";
    }*/
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
        Tools::log("OracleDB",QThread::currentThreadId(),QString("%1 连接失败，等待重连。。。").arg(m_strHostName));
        // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss :")<<m_strHostName+;
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
        // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<<<"OracleDB::ReConnectToDB";
        Tools::log("OracleDB",QThread::currentThreadId(),QString("数据库已重新连接"));
        m_bCheckConnectDB = false;
        return true;
    }
    Tools::log("OracleDB",QThread::currentThreadId(),QString("数据库连接失败"));
    // qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:")<<QString("数据库连接失败")<<"OracleDB::ReConnectToDB";

    m_bCheckConnectDB = true;
    return false;
}
