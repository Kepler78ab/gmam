#include "container.h"
#include <QApplication>
#include <QObject>
#include <windows.h>
#include<QFile>
#include<QDebug>
#include<QJsonDocument>
#include<QJsonObject>
#include<QLockFile>
#include<QDir>
#define CONFIG_INI "/./config.ini"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include "Structure.h"
#include <QProcess>
Container::Container(QObject *parent)
    : QObject{parent}
{

    // qDebug()<<"初始化";
    if(!init())
        return;

    // QTimer *timer=new QTimer(this);
    // connect(timer,&QTimer::timeout,this,&Container::slotOnRestart);
    // timer->start(10000);

}

void Container::setAutoStart(bool enable)
{
    QString appName = QCoreApplication::applicationName();
    QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if (enable) {
        settings.setValue(appName, appPath);
        QTextStream(stdout) << QString("已开启开机自启动\n");
    } else {
        settings.remove(appName);
        QTextStream(stdout) << QString("已关闭开机自启动\n");
    }
}





Container::~Container()
{

    qDebug()<<"container start to destroyed";
    if(m_app!=nullptr)
    {
        delete m_app;
    }
    m_app=nullptr;
    if(m_c!=nullptr)
    {
        delete m_c;
    }

    m_c=nullptr;
    this->disconnect();
    qDebug()<<"container destroyed";
}

void Container::slotOnQuit()
{
    qDebug()<<"退出...";
    HWND hWnd = GetConsoleWindow();

    ShowWindow(hWnd, SW_HIDE); // 显示
    // 提到最前

    qApp->quit();

}

void Container::slotOnRestart()
{
    qDebug()<<"清理开始";
    m_app->disconnect();
    m_c->disconnect();
    if(m_app)
    {
        // qDebug()<<"m_app 不为空";
        delete m_app;
    }
    if(m_c)
    {
        // qDebug()<<"m_c不为空";
        delete m_c;
    }
    m_c=nullptr;
    m_app=nullptr;
    // m_c=nullptr;
    // QTimer::singleShot(1000,t_c,&QObject::deleteLater);
    qDebug()<<"清理结束";
    QTimer::singleShot(2000,this,&Container::init);
}



bool Container::readConfig()
{
    // CONSOLE_CONFIG config;
    QString strFileName = QCoreApplication::applicationDirPath() + CONFIG_INI;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"open file "<<CONFIG_INI<<"failed!";
        return false;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isObject())
    {
        QJsonObject jsonObject = document.object();
        m_config.m_isConsole= jsonObject.value("isConsole").toBool();
        m_config.m_isGui= jsonObject.value("isGui").toBool();
        m_config.m_autoStart= jsonObject.value("autoStart").toBool();
        return true;
    }
    return true;
}

bool Container::readControllerConfig()
{
    QString strFileName = QCoreApplication::applicationDirPath() + CONFIG_INI;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"open file "<<CONFIG_INI<<"failed!";
        return false ;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isObject())
    {
        QJsonObject jsonObject = document.object();

        m_controllerConfig.m_dbConfig=getAbsolutePath(jsonObject.value("DBConfig").toString());

        m_controllerConfig.isThreadHandlePrint=jsonObject.value("isThreadHandlePrint").toBool();

        m_controllerConfig.isTimePrint=jsonObject.value("isTimePrint").toBool();

        m_controllerConfig.isModulePrint=jsonObject.value("isModulePrint").toBool();

        m_controllerConfig.isDBInit=jsonObject.value("InitDB").toBool();

        m_controllerConfig.isXlsxInit=jsonObject.value("InitXlsx").toBool();

        m_controllerConfig.isFileMoniterInit=jsonObject.value("InitFileMoniter").toBool();

        m_controllerConfig.isDebug=jsonObject.value("isDebug").toBool();

        QString unscanPath=getAbsolutePath(jsonObject.value("unScanFolderPath").toString());;
        QString unWritedPath=getAbsolutePath(jsonObject.value("unWritedFolderPath").toString());
        QString preparePath=getAbsolutePath(jsonObject.value("prepareFolderPath").toString());
        QString writedPath=getAbsolutePath(jsonObject.value("writedFolderPath").toString());


        if(!(checkPathExists(m_controllerConfig.m_dbConfig)
              &&checkPathExists(unscanPath)
              &&checkPathExists(unWritedPath)
              &&checkPathExists(preparePath)
              &&checkPathExists(writedPath)))
        {
            qDebug() << "某个路径不存在，检查配置文件请"; /*Tools::log(
                "Controller", QThread::currentThreadId(),
                "初始化失败，配置文件有问题");*/
            return false;
        }

        MONITER_CONFIG moniterConfig;
        moniterConfig.m_moduleName = jsonObject.value("moduleNameMointer").toString();
        moniterConfig.m_unScanFolderPath=unscanPath;
        moniterConfig.m_unWritedFolderPath=unWritedPath;
        moniterConfig.m_fileForm=jsonObject.value("FileForm").toString();
        moniterConfig.m_filter=jsonObject.value("Filter").toString();
        moniterConfig.m_filterRegEx=jsonObject.value("FilterRegEx").toString();
        moniterConfig.m_scanInterval=jsonObject.value("scanInterval").toInt();
        moniterConfig.m_waitInterval=jsonObject.value("waitInterval").toInt();

        XLSXR_CONFIG xlsxConfig;
        xlsxConfig.m_moduleName = jsonObject.value("moduleNameXlsx").toString();
        xlsxConfig.m_maxVal = jsonObject.value("maxVal").toDouble();
        xlsxConfig.m_minVal = jsonObject.value("minVal").toDouble();
        xlsxConfig.m_unit = jsonObject.value("unit").toString();
        xlsxConfig.m_formId = jsonObject.value("formId").toString();
        xlsxConfig.m_useCustomInterval = jsonObject.value("useCustomInteval").toBool();
        xlsxConfig.m_useCustomUnit = jsonObject.value("useCustomUnit").toBool();

        xlsxConfig.m_location_date=jsonObject.value("xls_date").toString();
        xlsxConfig.m_location_equipName=jsonObject.value("xls_equipName").toString();
        xlsxConfig.m_location_sampleName=jsonObject.value("xls_sampleName").toString();
        xlsxConfig.m_location_batchNum=jsonObject.value("xls_batchNum").toString();
        xlsxConfig.m_location_testLoad=jsonObject.value("xls_testLoad").toString();

        xlsxConfig.m_location_unit=jsonObject.value("xls_unit").toString();
        xlsxConfig.m_loaction_rawValue1=jsonObject.value("rawValues_1").toString();
        xlsxConfig.m_loaction_rawValue2=jsonObject.value("rawValues_2").toString();

        xlsxConfig.m_isDebugXlsData=jsonObject.value("isDebugXlsData").toBool();



        DISPATCHER_CONFIG dispatcherConfig;
        dispatcherConfig.m_moduleName = jsonObject.value("moduleNameDispatch").toString();
        dispatcherConfig.m_preparePath=preparePath;
        dispatcherConfig.m_writedPath=writedPath;
        dispatcherConfig.m_unWritedPath=unWritedPath;


        m_controllerConfig.m_moniterConfig=moniterConfig;
        m_controllerConfig.m_xlsxConfig=xlsxConfig;
        m_controllerConfig.m_dispatcherConfig=dispatcherConfig;

        m_statisticsPath=preparePath;
    }
    return true;
}

QString Container::getAbsolutePath(const QString &path)
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

bool Container::checkPathExists(const QString &path)
{
    QString absolutePath = getAbsolutePath(path);
    return QFileInfo::exists(absolutePath);
}

bool Container::init()
{
    // this->disconnect()
    if(!readConfig())
    {
        QTextStream(stdout)<<"初始化失败";
        return false;
    }

    if(!readControllerConfig())
    {
        QTextStream(stdout)<<"初始化失败";
        return false;
    }
    //----------------------------------------------------------
    // qDebug()<<"-------------------------";
    setAutoStart(m_config.m_autoStart);
    if(m_c)
    {
        delete m_c;
        m_c=nullptr;
    }
    m_c=new Controller(m_controllerConfig);
    if(m_config.m_isGui)
    {
        m_app=new Applic(m_config.m_isConsole,m_statisticsPath);
        HWND hWnd = GetConsoleWindow();
        if(m_config.m_isConsole)
        {
            ShowWindow(hWnd, SW_SHOW); // 显示
            SetForegroundWindow(hWnd); // 提到最前
        }
        else
            ShowWindow(hWnd, SW_HIDE);


        connect(m_c,SIGNAL(controllerStatus(bool)),m_app,SLOT(slotControllerStatus(bool)),Qt::QueuedConnection);
        connect(m_app,SIGNAL(requestQuit()),this,SLOT(slotOnQuit()),Qt::QueuedConnection);
        connect(m_app,&Applic::requestRestart,this,&Container::slotOnRestart);

        // QTimer::singleShot(10000, this, &Container::slotOnRestart);

        // qDebug()<<"app 初始化完成";
        return true;
    }
    else
    {
        return true;
    }

}

