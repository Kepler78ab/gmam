#include "Controller.h"
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QThread>
#include<QDebug>
#include<QFileInfo>
#include<QDir>
#include "Tools.h"
#define CONFIG_INI "/./config.ini"
Controller::Controller(CONTROLLER_CONFIG config,QObject *parent)
    : QObject{parent}
    ,m_config(config)
{
    init();
}
Controller::~Controller()
{

    disconnect();
    // this->disconnect();
    qDebug()<<"contoller start destroy";
    // this->disconnect();

    // if (m_dbThread) {
    //     m_dbThread->quit();
    //     if (!m_dbThread->wait(300)) { // 给3秒等待时间
    //         m_dbThread->terminate();
    //     }
    // }
    // if (m_xlsxThread) {
    //     m_xlsxThread->quit();
    //     if (!m_xlsxThread->wait(300)) { // 给3秒等待时间
    //         m_xlsxThread->terminate();
    //          delete m_xlsxThread;

    //     }
    // }
    // if (m_dispatcherThread) {
    //     m_dispatcherThread->quit();
    //     if (!m_dispatcherThread->wait(300)) { // 给3秒等待时间
    //         m_dispatcherThread->terminate();
    //         delete m_dispatcherThread;
    //     }
    // }
    // if (m_mointerThread) {
    //     m_mointerThread->quit();
    //     if (!m_mointerThread->wait(300)) { // 给3秒等待时间
    //         m_mointerThread->terminate();
    //         delete m_mointerThread;
    //     }
    // }
    // m_dbThread->quit();
    // m_dbThread->wait();

    // m_xlsxThread->quit();
    // m_xlsxThread->wait();

    // m_dispatcherThread->quit();
    // m_dispatcherThread->wait();

    // m_mointerThread->quit();
    // m_mointerThread->wait();


    if (m_moniter) {
        QMetaObject::invokeMethod(m_moniter, "slotStop", Qt::BlockingQueuedConnection);
    }
    m_moniter->disconnect();
    m_xlsxReader->disconnect();
    m_db->disconnect();
    m_dispatcher->disconnect();

    m_dbThread->quit();
    m_xlsxThread->quit();
    m_dispatcherThread->quit();
    m_mointerThread->quit();
    // m_dbThread->quit();


    m_xlsxThread->wait();

    m_dispatcherThread->wait();

    m_mointerThread->wait();
     m_dbThread->wait();


    // if(!m_dbThread->wait(2000)) {
    //     m_dbThread->terminate(); // 3秒还没停，强制终止（下策）
    //     qDebug()<<"thread stop";
    //     m_dbThread->wait();
    // }
    // if(m_dbThread)
    //     delete m_dbThread;

    // m_xlsxThread->deleteLater();

    // m_dispatcherThread->deleteLater();

    // m_mointerThread->deleteLater();

    // m_dbThread->deleteLater();

    // if(m_dispatcher)
    //     delete m_dispatcher;
    // if(m_xlsxReader)
    //     delete m_xlsxReader;
    // if(m_db)
    //     delete m_db;
    // if(m_moniter)
    //     delete m_moniter;

    delete m_dbThread;;
    delete m_xlsxThread;

    delete m_dispatcherThread;

    delete m_mointerThread;

    m_dispatcherThread=nullptr;
    m_xlsxThread=nullptr;
    // m_dbThread=nullptr;
    m_mointerThread=nullptr;
    delete m_moniter;

    m_moniter=nullptr;
    m_dispatcher=nullptr;
    m_xlsxReader=nullptr;
    m_db=nullptr;
    // this->disconnect();

    qDebug()<<"contoller destroyed";
}
// bool Controller::readConfig()
// {
//     QString strFileName = QCoreApplication::applicationDirPath() + CONFIG_INI;
//     QFile file(strFileName);
//     if(!file.open((QIODevice::ReadOnly)))
//     {
//         qDebug()<<"open file "<<CONFIG_INI<<"failed!";
//         return false ;
//     }
//     QByteArray jsonData = file.readAll();
//     file.close();
//     QJsonDocument document = QJsonDocument::fromJson(jsonData);
//     if(document.isObject())
//     {
//         QJsonObject jsonObject = document.object();

//         m_dbConfig=getAbsolutePath(jsonObject.value("DBConfig").toString());

//         isThreadHandlePrint=jsonObject.value("isThreadHandlePrint").toBool();

//         isTimePrint=jsonObject.value("isTimePrint").toBool();

//         isModulePrint=jsonObject.value("isModulePrint").toBool();

//         isDBInit=jsonObject.value("InitDB").toBool();

//         isXlsxInit=jsonObject.value("InitXlsx").toBool();

//         isFileMoniterInit=jsonObject.value("InitFileMoniter").toBool();

//         isDebug=jsonObject.value("isDebug").toBool();

//         QString unscanPath=getAbsolutePath(jsonObject.value("unScanFolderPath").toString());;
//         QString unWritedPath=getAbsolutePath(jsonObject.value("unWritedFolderPath").toString());
//         QString preparePath=getAbsolutePath(jsonObject.value("prepareFolderPath").toString());
//         QString writedPath=getAbsolutePath(jsonObject.value("writedFolderPath").toString());


//         if(!(checkPathExists(m_dbConfig)
//               &&checkPathExists(unscanPath)
//               &&checkPathExists(unWritedPath)
//               &&checkPathExists(preparePath)
//               &&checkPathExists(writedPath)))
//         {
//             qDebug() << "某个路径不存在，检查配置文件请"; /*Tools::log(
//                 "Controller", QThread::currentThreadId(),
//                 "初始化失败，配置文件有问题");*/
//             return false;
//         }

//         m_moniterConfig.m_moduleName = jsonObject.value("moduleNameMointer").toString();
//         m_moniterConfig.m_unScanFolderPath=unscanPath;
//         m_moniterConfig.m_unWritedFolderPath=unWritedPath;
//         m_moniterConfig.m_fileForm=jsonObject.value("FileForm").toString();
//         m_moniterConfig.m_filter=jsonObject.value("Filter").toString();
//         m_moniterConfig.m_filterRegEx=jsonObject.value("FilterRegEx").toString();
//         m_moniterConfig.m_scanInterval=jsonObject.value("scanInterval").toInt();
//         m_moniterConfig.m_waitInterval=jsonObject.value("waitInterval").toInt();

//         m_xlsxConfig.m_moduleName = jsonObject.value("moduleNameXlsx").toString();
//         m_xlsxConfig.m_maxVal = jsonObject.value("maxVal").toDouble();
//         m_xlsxConfig.m_minVal = jsonObject.value("minVal").toDouble();
//         m_xlsxConfig.m_unit = jsonObject.value("unit").toString();
//         m_xlsxConfig.m_formId = jsonObject.value("formId").toString();
//         m_xlsxConfig.m_useCustomInterval = jsonObject.value("useCustomInteval").toBool();

//         m_dispatcherConfig.m_moduleName = jsonObject.value("moduleNameDispatch").toString();
//         m_dispatcherConfig.m_preparePath=preparePath;
//         m_dispatcherConfig.m_writedPath=writedPath;
//         m_dispatcherConfig.m_unWritedPath=unWritedPath;
//     }
//     return true;
// }

bool Controller::init()
{
    Tools::setConfig(m_config.isThreadHandlePrint,m_config.isTimePrint,m_config.isModulePrint,m_config.isDebug);

    Tools::log("Controller",QThread::currentThreadId(),"初始化...");
    m_xlsxThread=new QThread();
    m_dbThread=new QThread();
    m_dispatcherThread =new QThread();
    m_mointerThread =new QThread();

    m_dispatcher = new FileDispatcher(m_config.m_dispatcherConfig);
    m_dispatcher->moveToThread(m_dispatcherThread);
    connect(m_dispatcherThread,&QThread::finished,m_dispatcher,&QObject::deleteLater);
    connect(m_dispatcherThread,&QThread::finished,m_dispatcherThread,&QObject::deleteLater);


    // 配置分发器路径


    m_moniter=new FileMoniter(m_config.m_moniterConfig);
    m_moniter->moveToThread(m_mointerThread);
    connect(m_moniter,SIGNAL(requestMove(QString,int)),m_dispatcher,SLOT(slotMoveFile(QString,int)));
    connect(m_dispatcher, &FileDispatcher::moveFailed, m_moniter, &FileMoniter::slotRemoveFromProcessed);
    connect(m_mointerThread,&QThread::started,m_moniter,&FileMoniter::slotStart);
    // connect(m_mointerThread,&QThread::finished,m_moniter,&QObject::deleteLater);
    // connect(m_mointerThread,&QThread::finished,m_mointerThread,&QObject::deleteLater);
    Tools::log("Controller",QThread::currentThreadId(),"检测文件模块初始化");


    m_xlsxReader=new ExcelOperator(m_config.m_xlsxConfig);
    m_xlsxReader->moveToThread(m_xlsxThread);
    connect(m_dispatcher, SIGNAL(moveSuccess(const QString&,int)), m_xlsxReader,SLOT(processExecl(const QString&,int)));
    connect(m_xlsxThread,&QThread::finished,m_xlsxReader,&QObject::deleteLater);
    connect(m_xlsxThread,&QThread::finished,m_xlsxThread,&QObject::deleteLater);

    Tools::log("Controller",QThread::currentThreadId(),"读xlsx模块初始化");



    OracleDB::setConfig(m_config.m_dbConfig);
    m_db=new OracleDB();


    m_db->moveToThread(m_dbThread);
    connect(m_dbThread,&QThread::started,m_db,&OracleDB::slotInit);
    connect(m_dbThread,&QThread::finished,m_db,&QObject::deleteLater);
    connect(m_dbThread,&QThread::finished,m_dbThread,&QObject::deleteLater);

    // m_db->moveToThread(m_mointerThread);
    // connect(m_mointerThread,&QThread::started,m_db,&OracleDB::slotInit);
    // connect(m_mointerThread,&QThread::finished,m_db,&QObject::deleteLater);
    // connect(m_mointerThread,&QThread::finished,m_mointerThread,&QObject::deleteLater);
    connect(m_xlsxReader,SIGNAL(sendData(const QByteArray&,const QString&)),m_db,SLOT(slotRecvData(const QByteArray&,const QString&)));


    connect(m_db, &OracleDB::dataWrited, m_dispatcher, [this](const QString &path){
        m_dispatcher->slotMoveFile(path, 1);
    });connect(m_db, &OracleDB::dataNotWrited, m_dispatcher, [this](const QString &path){
        m_dispatcher->slotMoveFile(path, 2);
    });
    connect(m_db,SIGNAL(dbStatusSignal(bool)), m_dispatcher,SLOT(slotDbStatusChanged(bool)));
    connect(m_db,SIGNAL(dbStatusSignal(bool)),m_moniter,SLOT(slotDbStatusChanged(bool)));
    connect(m_db,&OracleDB::dbStatusSignal,[this](bool s){
        if(s!=m_config.isControllerOnline)
        {
            m_config.isControllerOnline=s;
            emit controllerStatus(s);
        }
    });
    Tools::log("Controller",QThread::currentThreadId(),"数据库模块初始化");

    m_mointerThread->start();
    m_dispatcherThread->start();
    m_dbThread->start();
    m_xlsxThread->start();

    return true;
}

QString Controller::getAbsolutePath(const QString &path)
{
    QFileInfo fileInfo(path);

    // 如果已经是绝对路径，直接返回
    if (fileInfo.isAbsolute()) {
        return QDir::cleanPath(path);
    }

    // 相对路径：拼接应用目录
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = QDir(basePath).absoluteFilePath(path);

    return QDir::cleanPath(fullPath);
}

bool Controller::checkPathExists(const QString &path)
{
    QString absolutePath = getAbsolutePath(path);
    return QFileInfo::exists(absolutePath);
}



