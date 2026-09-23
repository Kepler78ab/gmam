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
FileMoniter *FileMoniter::m_instance=nullptr;
QString FileMoniter::m_configPath=NULL;
FileMoniter::FileMoniter()
    : QThread()
{
    readJson();
    connect(&m_watcher,SIGNAL(directoryChanged(QString)),this,SLOT(slotDirectoryChanged(QString)),Qt::DirectConnection);
}

FileMoniter::~FileMoniter()
{
    qDebug()<<"FileMonitor 析构";
}

FileMoniter *FileMoniter::instance()
{
    if (!m_instance) {
        m_instance = new FileMoniter();
    }
    return m_instance;
}
void FileMoniter::readJson()
{
    QString strConfigPath = m_configPath;
    QFile file(strConfigPath);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            m_unScanFolderPath = Tools::getAbsolutePath(obj.value("unScanFolderPath").toString());
            m_unWritedFolderPath = Tools::getAbsolutePath(obj.value("unWritedFolderPath").toString());
            m_scanInterval = obj.value("scanInterval").toInt();
            m_waitInterval = obj.value("waitInterval").toInt();
            m_filter=obj.value("Filter").toString();
            m_fileForm=obj.value("FileForm").toString();
            m_filterRegEx=obj.value("FilterRegEx").toString();
            if(!(Tools::checkPathExists(m_unScanFolderPath)&&Tools::checkPathExists(m_unWritedFolderPath)))
                Tools::log("FileOper",QThread::currentThreadId(),"源路径和目标路径存在问题");
        }
    }
    Tools::log("FileOper",QThread::currentThreadId(),"文件检测读配置完成");
}

QStringList FileMoniter::scanFiles(const QString& folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return QStringList();
    }

    // 获取目录下所有文件
    QStringList scanFiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
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
    QStringList newEntryList = allFiles.filter(m_fileForm);
    QRegExp re(m_filterRegEx);
    QStringList validFiles = newEntryList.filter(re);
    return validFiles;
}

bool FileMoniter::processSingleFile(const QString &fileName)
{
    // QString srcPath = fileName;
    // QString dstPath = m_prepareFolderPath + "/" + QFileInfo(fileName).fileName();

    // 检查文件是否就绪
    if (!isFileReady(fileName)) {
        Tools::log("FileOper", QThread::currentThreadId(),
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
    // qDebug()<<fileName;

    // 处理目标文件已存在的情况
    // int counter = 1;
    // while (QFile::exists(dstPath)) {
    //     QString baseName = fileName;
    //     int dotIndex = baseName.lastIndexOf('.');
    //     if (dotIndex > 0) {
    //         baseName = baseName.left(dotIndex) +
    //                    QString("_%1").arg(counter) +
    //                    baseName.mid(dotIndex);
    //     } else {
    //         baseName = baseName + QString("_%1").arg(counter);
    //     }
    //     dstPath = m_unWritedFolderPath + "/" + baseName;
    //     counter++;
    // }

    // 移动文件
    // if (QFile::rename(srcPath, dstPath)) {
    //     Tools::log("FileOper", QThread::currentThreadId(),QString("移动文件到待读取文件夹: %1").arg(fileName));

    //     // 记录已处理
    //     {
    //         QMutexLocker locker(&m_processMutex);
    //         m_processedFiles.insert(fileName);
    //     }

    //     // 等待文件稳定
    //     if (m_waitInterval > 0) {
    //         QThread::sleep(m_waitInterval);
    //     }

    //     // 发射信号
    //     emit getData(dstPath);

    //     return true;
    // } else {
    //     Tools::log("FileOper", QThread::currentThreadId(),
    //                QString("移动文件失败: %1").arg(fileName));
    //     return false;
    // }
    return true;
}

void FileMoniter::processAllNewFile(const QString& part)
{
    // QStringList targetFolders;
    // targetFolders << m_unScanFolderPath << m_unWritedFolderPath;

    // for (const QString& folderPath : targetFolders) {
    //     // scanFiles 返回该文件夹下所有符合过滤条件的绝对路径列表
    //     QStringList filePaths = scanFiles(folderPath);

    //     for (const QString& path : filePaths) {
    //         processSingleFile(path);
    //     }
    // }
    QStringList newFiles=scanFiles(m_unScanFolderPath)+scanFiles(m_unWritedFolderPath);
    // QStringList newFiles=scanFiles(m_un);
    if(newFiles.isEmpty())
    {
        // Tools::log("FileOper", QThread::currentThreadId(), "没有新文件需要处理");
        return;
    }
    Tools::log("FileOper", QThread::currentThreadId(),QString("%1发现 %2 个新文件").arg(part).arg(newFiles.size()));
    // qDebug()<<"文件:"<<newFiles;
    int processedCount = 0;
    int failedCount = 0;

    // 处理每个文件
    for (const QString& file : newFiles) {
        if (processSingleFile(file)) {
            emit requestMove(file, 0);
            //处理一个文件后短暂休眠避免IO压力过大
            QThread::msleep(m_waitInterval);
            // qDebug()<<"休眠"<<m_waitInterval<<"ms";
            processedCount++;
        } else {
            failedCount++;
        }   
    }
    // // 记录处理结果
    if (processedCount > 0 || failedCount > 0) {
        Tools::log("FileOper", QThread::currentThreadId(),QString("交给dispatcher处理: 成功 %1, 失败 %2").arg(processedCount).arg(failedCount));
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
    if(path!=m_unScanFolderPath)
        return;
    emit immediateProcess();
    // m_hasChanges=true;
    // m_hasChanges.store(true, std::memory_order_release);
}


void FileMoniter::slotRemoveFromProcessed(const QString &filename)
{
    QMutexLocker locker(&m_processMutex);
    m_processedFiles.remove(filename);
    Tools::log("FileMoniter", QThread::currentThreadId(), "已从处理列表中移除(重置): " + filename);
}
void FileMoniter::run()
{
    if (m_unScanFolderPath.isEmpty() || !QDir(m_unScanFolderPath).exists()) {
        Tools::log("FileMoniter-Error", QThread::currentThreadId(), "监控路径无效");
        return;
    }
    m_watcher.addPath(m_unScanFolderPath);

    processAllNewFile("定时扫描");
    cleanupProcessRecord();
    QEventLoop eventLoop;
    QTimer scanTimer;
    //定时扫描
    scanTimer.setInterval(m_scanInterval * 1000);
    connect(&scanTimer, &QTimer::timeout, this, [this]() {
        // Tools::log("FileOper", QThread::currentThreadId(), "定时扫描");
        processAllNewFile("定时扫描");
        cleanupProcessRecord();
    });
    scanTimer.start();

    // 立即处理请求
    connect(this, &FileMoniter::immediateProcess, this, [this]() {
        // Tools::log("FileOper", QThread::currentThreadId(), "被动扫描");
        processAllNewFile("被动扫描");
        cleanupProcessRecord();
    }, Qt::QueuedConnection);

    connect(this, &FileMoniter::finished, &eventLoop, &QEventLoop::quit);
    // qDebug() << "[监控线程] 事件循环启动";
    eventLoop.exec();

    // qDebug() << "[监控线程] 事件循环结束";
}

void FileMoniter::setConfig(QString filePath)
{
    m_configPath=filePath;
}

void FileMoniter::stop()
{
    this->quit();
    if(!this->wait(3000)) {
        this->terminate();
    }
}

