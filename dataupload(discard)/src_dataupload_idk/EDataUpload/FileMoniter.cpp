#include "FileMoniter.h"
#include "Tools.h"
#include <QDebug>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QMutexLocker>
#include<QObject>
FileMoniter::FileMoniter(const MONITER_CONFIG& config,QObject *parent)
    :QObject(parent),m_config(config)
{

    // m_unScanFolderPath=uscan;
    // m_unWritedFolderPath=unwrited;

    // m_fileForm=fileform;
    // m_filter=filter;
    // m_filterRegEx=regEx;

    // m_scanInterval=scanInterval;
    // m_waitInterval=waitInterval;
    // readJson();


    // init();

    m_scanTimer=nullptr;

}

FileMoniter::~FileMoniter()
{
    qDebug() << "fileMointer close";

    // // 停止所有定时器
    // if (m_scanTimer) {
    //     m_scanTimer->stop();
    //     m_scanTimer->deleteLater();
    //     m_scanTimer = nullptr;
    // }

    // // 断开所有连接
    // disconnect();

    // // 清理文件监控
    // if (!m_config.m_unScanFolderPath.isEmpty()) {
    //     m_watcher.removePath(m_config.m_unScanFolderPath);
    // }

    // // 清空处理记录
    // cleanupProcessRecord();

   qDebug()<<"fileMointer destroy";
}

QStringList FileMoniter::scanFiles(const QString& folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return QStringList();
    }

    // 获取目录下所有文件
    QStringList scanFiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    // qDebug()<<"扫了"<<scanFiles.size()<<"个文件";
    if (scanFiles.isEmpty()) {
        return QStringList();
    }

    // 过滤有效文件
    QStringList validFiles = filterValidFiles(scanFiles);
    // 过滤掉已处理的文件
    QStringList newFiles;
    {
        QMutexLocker locker(&m_processMutex);
        for (const QString& file : validFiles) {
            if (!m_processedFiles.contains(file)) {
                newFiles.append(folderPath+"/"+file);
            }
        }
    }
    return newFiles;
}

QStringList FileMoniter::filterValidFiles(const QStringList &allFiles) const
{
    QStringList newEntryList = allFiles.filter(m_config.m_fileForm);
    QRegExp re(m_config.m_filterRegEx);
    QStringList validFiles = newEntryList.filter(re);
    return validFiles;
}

bool FileMoniter::processSingleFile(const QString &fileName)
{
    // QString srcPath = fileName;
    // QString dstPath = m_prepareFolderPath + "/" + QFileInfo(fileName).fileName();

    // 检查文件是否就绪
    if (!isFileReady(fileName)) {
        Tools::log(m_config.m_moduleName, QThread::currentThreadId(),
                   QString("文件未就绪: %1").arg(fileName));
        return false;
    }
    {
        QMutexLocker locker(&m_processMutex);
        if (m_processedFiles.contains(fileName)) {
            return false;
        }
        m_processedFiles.insert(fileName); // 占位
    }
    return true;
}

void FileMoniter::processAllNewFile(const QString& part)
{
    QStringList newFiles;
    newFiles.append(scanFiles(part));

    // QStringList newFiles=scanFiles(m_un);
    if(newFiles.isEmpty())
    {
        // Tools::log(m_config.m_moduleName, QThread::currentThreadId(), "没有新文件需要处理");
        return;
    }
    Tools::log(m_config.m_moduleName, QThread::currentThreadId(),QString("%1发现 %2 个新文件").arg(part==m_config.m_unScanFolderPath?"检测":"定时检测").arg(newFiles.size()));
    // qDebug()<<"文件:"<<newFiles;
    int processedCount = 0;
    int failedCount = 0;

    // 处理每个文件
    for (const QString& file : newFiles) {
        if (processSingleFile(file)) {
            emit requestMove(file, 0);
            //处理一个文件后短暂休眠避免IO压力过大
            QThread::msleep(m_config.m_waitInterval);
            // qDebug()<<"休眠"<<m_waitInterval<<"ms";
            processedCount++;
        } else {
            failedCount++;
        }
    }
    // // 记录处理结果
    if (processedCount > 0 || failedCount > 0) {
        Tools::log(m_config.m_moduleName, QThread::currentThreadId(),QString("交给分发模块处理文件 %1 ,未交付 %2").arg(processedCount).arg(failedCount));
    }
}

void FileMoniter::cleanupProcessRecord()
{
    QMutexLocker locker(&m_processMutex);
    m_processedFiles.clear();
}

bool FileMoniter::isFileReady(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }
    if (fileInfo.size() == 0) {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    file.close();
    return true;
}


void FileMoniter::slotDirectoryChanged(const QString &path)
{
    if(path!=m_config.m_unScanFolderPath)
        return;
    if(!m_isDbOnline)
        return;
    /*
        if (!m_isMoniterRun) {
            return; // 冷却中，直接拦截
        }
        m_isMoniterRun=false;*/

    // qDebug()<<QDateTime::currentDateTime();
    QTimer::singleShot(m_config.m_waitInterval, this, [this](){
        // m_isMoniterRun=true;
        emit immediateProcess();
        // qDebug()<<QDateTime::currentDateTime();
    });
    // return;


}

void FileMoniter::slotRemoveFromProcessed(const QString &filename)
{
    QMutexLocker locker(&m_processMutex);
    m_processedFiles.remove(filename);
    Tools::log(m_config.m_moduleName, QThread::currentThreadId(), "已从处理列表中移除(重置): " + filename);
}

void FileMoniter::slotDbStatusChanged(bool isDBonline)
{
    m_isDbOnline=isDBonline;
    // if(!isDBonline)
    //     m_isMoniterRun=isDBonline;
    // else
    //     m_isMoniterRun=isDBonline;

}

void FileMoniter::slotStart()
{
    if(!m_scanTimer) init();
}

void FileMoniter::slotStop()
{
    disconnect();
    qDebug() << "FileMoniter stopping...";

    // 停止定时器
    if (m_scanTimer) {
        m_scanTimer->stop();
        // m_scanTimer->deleteLater();
        delete m_scanTimer;
        m_scanTimer = nullptr;
    }

    // 移除文件监控
    if (!m_config.m_unScanFolderPath.isEmpty()) {
        m_watcher.removePath(m_config.m_unScanFolderPath);
    }

    // 设置标志位
    m_isMoniterRun = false;

    // 清理处理记录
    cleanupProcessRecord();

    qDebug() << "FileMoniter stopped";
}

void FileMoniter::slotAutoScan()
{
    if(!m_isDbOnline)
    {
        return;
    }

    processAllNewFile(m_config.m_unScanFolderPath);
    //"被动"
    processAllNewFile(m_config.m_unWritedFolderPath);
    cleanupProcessRecord();
}

void FileMoniter::slotPassiveScan()
{
    if(!m_isDbOnline)
    {
        return;
    }
    processAllNewFile(m_config.m_unScanFolderPath);
    cleanupProcessRecord();
}

void FileMoniter::init()
{
    m_watcher.addPath(m_config.m_unScanFolderPath);
    m_scanTimer=new QTimer();
    //定时扫描
    m_scanTimer->setInterval(m_config.m_scanInterval * 1000);
    connect(m_scanTimer, &QTimer::timeout,this,&FileMoniter::slotAutoScan);
    // 立即处理请求
    connect(this, &FileMoniter::immediateProcess,this,&FileMoniter::slotPassiveScan);
    connect(&m_watcher,SIGNAL(directoryChanged(QString)),this,SLOT(slotDirectoryChanged(QString)));
    m_scanTimer->start();
    //"定时扫描"

}


