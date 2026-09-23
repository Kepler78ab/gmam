#include "FileDispatcher.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "Tools.h"
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
QString FileDispatcher::m_configPath=NULL;
FileDispatcher::FileDispatcher(QObject *parent) : QObject(parent), m_isDbOnline(true) {

    readJson();
}

void FileDispatcher::setConfig(const QString &path)
{
    m_configPath=path;
}

void FileDispatcher::slotDbStatusChanged(bool online) {
    m_isDbOnline = online;
    Tools::log("Dispatcher", QThread::currentThreadId(),QString("同步数据库状态: %1").arg(online ? "在线" : "离线"));
}

void FileDispatcher::slotMoveFile(const QString &srcPath, int type)
{
    QFileInfo info(srcPath);
    QString fileName = info.fileName();
    QString targetPath;
    // qDebug()<<"slotMoveFIle";
    // 数据库离线
    if (!m_isDbOnline && type == 0) {
        Tools::log("Dispatcher", QThread::currentThreadId(), "DB离线，挂起移动操作");
        return;
    }
// qDebug()<<"swtich";
    switch (type) {
    case 0: targetPath = m_preparePath + "/" + QFileInfo(fileName).fileName(); break;  // 移动到准备区
    case 1: targetPath = m_writedPath + "/" + QFileInfo(fileName).fileName(); break;   // 成功归档
    case 2: targetPath = m_unWritedPath + "/" + QFileInfo(fileName).fileName(); break; // 失败待查
    }

    if (performMove(srcPath, targetPath)) {
        emit moveSuccess(targetPath, type);
    } else {
        emit moveFailed(srcPath, "Rename failed");
    }
}

bool FileDispatcher::performMove(const QString &src, const QString &dst)
{
    if (!QFile::exists(src)) return false;

    //重名
    QString finalDst = dst;
    int counter = 1;
    while (QFile::exists(finalDst)) {
        QFileInfo fi(dst);
        finalDst = fi.absolutePath() + "/" + fi.baseName() +
                   QString("_%1.").arg(counter++) + fi.completeSuffix();
    }


    if (QFile::rename(src, finalDst)) {
        QFileInfo info(src);
        Tools::log("Dispatcher", QThread::currentThreadId(),
                   QString("移动文件 %1 : %2 -> %3")
                       .arg(QFileInfo(src).fileName())
                       .arg(QFileInfo(src).absoluteDir().dirName())
                       .arg(QFileInfo(finalDst).absoluteDir().dirName()));
        return true;
    }
    return false;
}

void FileDispatcher::readJson()
{
    QString strConfigPath = m_configPath;
    QFile file(strConfigPath);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            m_unWritedPath = Tools::getAbsolutePath(obj.value("unWritedFolderPath").toString());
            m_preparePath = Tools::getAbsolutePath(obj.value("prepareFolderPath").toString());
            m_writedPath = Tools::getAbsolutePath(obj.value("writedFolderPath").toString());
            if(!(Tools::checkPathExists(m_unWritedPath)&&Tools::checkPathExists(m_preparePath)&&Tools::checkPathExists(m_writedPath)))
                Tools::log("FileDisp",QThread::currentThreadId(),"源路径和目标路径存在问题");
        }
    }
    Tools::log("FileDisp",QThread::currentThreadId(),"文件检测读配置完成");
}
