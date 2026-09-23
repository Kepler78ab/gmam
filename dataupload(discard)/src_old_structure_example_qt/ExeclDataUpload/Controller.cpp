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
Controller::Controller(QObject *parent)
    : QObject{parent}
{
    readConfig();
    init();
}
Controller::~Controller()
{
    m_dbThread->quit();
    m_dbThread->wait();

    m_xlsxThread->quit();
    m_xlsxThread->wait();

    m_dispatcherThread->quit();
    m_dispatcherThread->wait();

    m_fileMoniterThread->stop();
    // m_fileMoniterThread->wait();
    delete m_fileMoniterThread;

    m_dispatcherThread=nullptr;
    m_xlsxThread=nullptr;
    m_dbThread=nullptr;
    m_fileMoniterThread=nullptr;
    m_dispatcher=nullptr;
    m_xlsxReader=nullptr;
    m_db=nullptr;

}
void Controller::readConfig()
{
    QString strFileName = QCoreApplication::applicationDirPath() + CONFIG_INI;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"open file "<<CONFIG_INI<<"failed!";
        return ;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isObject())
    {
        QJsonObject jsonObject = document.object();

        m_dbConfig=Tools::getAbsolutePath(jsonObject.value("DBConfig").toString());

        m_readFileConfig =Tools::getAbsolutePath(jsonObject.value("ReadFileConfig").toString());

        m_fileMointerConfig =Tools::getAbsolutePath(jsonObject.value("FileMointerConfig").toString());

        isThreadHandlePrint=jsonObject.value("isThreadHandlePrint").toBool();

        isTimePrint=jsonObject.value("isTimePrint").toBool();

        isModulePrint=jsonObject.value("isModulePrint").toBool();

        isDBInit=jsonObject.value("InitDB").toBool();

        isXlsxInit=jsonObject.value("InitXlsx").toBool();

        isFileMoniterInit=jsonObject.value("InitFileMoniter").toBool();

        //path
        // m_unWritedFolderPath = Tools::getAbsolutePath(jsonObject.value("unWritedFolderPath").toString());
        // m_prepareFolderPath = Tools::getAbsolutePath(jsonObject.value("prepareFolderPath").toString());
        // m_writedFolderPath = Tools::getAbsolutePath(jsonObject.value("writedFolderPath").toString());
    }
}

void Controller::init()
{
    Tools::setConfig(isThreadHandlePrint,isTimePrint,isModulePrint);

    Tools::log("Controller",QThread::currentThreadId(),"初始化...");
    if(!(Tools::checkPathExists(m_dbConfig)&&Tools::checkPathExists(m_fileMointerConfig)&&Tools::checkPathExists(m_readFileConfig)))
    {
        Tools::log("Controller",QThread::currentThreadId(),"初始化失败，配置文件有问题");
        return;
    }
    m_xlsxThread=new QThread();
    m_dbThread=new QThread();
    // m_dbThread=new QThread();
    FileDispatcher::setConfig(m_fileMointerConfig);
    m_dispatcherThread =new QThread();
    m_dispatcher = new FileDispatcher();
    connect(m_dispatcherThread,&QThread::finished,m_dispatcherThread,&QObject::deleteLater);
    connect(m_dispatcherThread,&QThread::finished,m_dispatcher,&QObject::deleteLater);

    // 配置分发器路径
    m_dispatcher->moveToThread(m_dispatcherThread);
    if(isFileMoniterInit)
    {
        FileMoniter::setConfig(m_fileMointerConfig);
        m_fileMoniterThread=FileMoniter::instance();
        connect(m_fileMoniterThread,SIGNAL(requestMove(QString,int)),m_dispatcher,SLOT(slotMoveFile(QString,int)),Qt::QueuedConnection);
        connect(m_dispatcher, &FileDispatcher::moveFailed, m_fileMoniterThread, &FileMoniter::slotRemoveFromProcessed);

        Tools::log("Controller",QThread::currentThreadId(),"检测文件模块初始化");
    }
    if(isFileMoniterInit&&isXlsxInit)
    {
        ExcelOperator::setConfig(m_readFileConfig);
        m_xlsxReader=new ExcelOperator();
        m_xlsxReader->moveToThread(m_xlsxThread);
        connect(m_dispatcher, &FileDispatcher::moveSuccess, this, [=](const QString &newPath, int type){
            if (type == 0) {
                m_xlsxReader->readFile(newPath);
            }
        });
        // connect(m_fileMoniterThread,SIGNAL(getData(QString)),m_xlsxReader,SLOT(readFile(QString)),Qt::QueuedConnection);
        connect(m_xlsxThread,&QThread::finished,m_xlsxReader,&QObject::deleteLater);
        connect(m_xlsxThread,&QThread::finished,m_xlsxThread,&QObject::deleteLater);

        Tools::log("Controller",QThread::currentThreadId(),"读xlsx模块初始化");
    }
    if(isDBInit&&isXlsxInit)
    {
        OracleDB::setConfig(m_dbConfig);
        m_db=new OracleDB();
        m_db->moveToThread(m_dbThread);
        connect(m_xlsxReader,SIGNAL(sendData(QByteArray,QString)),m_db,SLOT(slotRecvData(QByteArray,QString)),Qt::QueuedConnection);
        // connect(m_dbThread,&QThread::started,m_db,&OracleDB::startReconnectTimer);
        connect(m_dbThread,&QThread::finished,m_db,&QObject::deleteLater);
        connect(m_dbThread,&QThread::finished,m_dbThread,&QObject::deleteLater);
        connect(m_db, &OracleDB::dataWrited, m_dispatcher, [=](const QString &path){
            m_dispatcher->slotMoveFile(path, 1);
        });connect(m_db, &OracleDB::dataNotWrited, m_dispatcher, [=](const QString &path){
            m_dispatcher->slotMoveFile(path, 2);
        });
        connect(m_db,SIGNAL(dbStatusSignal(bool)), m_dispatcher,SLOT(slotDbStatusChanged(bool)),Qt::QueuedConnection);
        Tools::log("Controller",QThread::currentThreadId(),"数据库模块初始化");
    }
    m_dispatcherThread->start();
    m_dbThread->start();
    m_xlsxThread->start();
    m_fileMoniterThread->start();



}

