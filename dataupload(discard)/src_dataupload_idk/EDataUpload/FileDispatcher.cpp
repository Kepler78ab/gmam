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
FileDispatcher::FileDispatcher(const DISPATCHER_CONFIG config,QObject *parent) : QObject(parent),m_config(config),m_isDbOnline(true),m_isPrint(true) {
}

FileDispatcher::~FileDispatcher()
{
    qDebug()<<"fileDispatcher destroy";
}
void FileDispatcher::slotDbStatusChanged(bool online) {
    m_isDbOnline = online;
    if(online) m_isPrint=true;
    // Tools::log("Dispatcher", QThread::currentThreadId(),QString("同步数据库状态: %1").arg(online ? "在线" : "离线"));
}

void FileDispatcher::slotMoveFile(const QString &srcPath, int type)
{
    QFileInfo info(srcPath);
    QString fileName = info.fileName();
    QString targetPath;
    // qDebug()<<"slotMoveFIle";
    // 数据库离线
    if (!m_isDbOnline && type == 0) {
        if(m_isDbOnline!=m_isPrint)
        {
            Tools::log(m_config.m_moduleName, QThread::currentThreadId(), "DB离线，挂起移动操作");
            m_isPrint=false;
        }
        return;
    }
    // m_isPrint=true;
    // qDebug()<<".swtich";
    switch (type) {
    case 0:
        // targetPath = m_config.m_preparePath + "/" + QFileInfo(fileName).fileName();
        targetPath = m_config.m_preparePath + "/" + QFileInfo(fileName).completeBaseName()+"_p."+ QFileInfo(fileName).suffix();
        //加_r

        break;  // 移动到准备区
    case 1:
        targetPath =  m_config.m_writedPath + "/" + QFileInfo(fileName).completeBaseName()+"_r."+ QFileInfo(fileName).suffix();
        // targetPath =  m_config.m_writedPath + "/" + QFileInfo(fileName).completeBaseName()+"_r"+ QFileInfo(fileName).suffix();
        //加_p
        break;   // 成功归档
    case 2:
        if(!QFileInfo(fileName).completeBaseName().endsWith("_p"))
        {
            return;
        }
        QString rBaseName=QFileInfo(fileName).completeBaseName();
        rBaseName.chop(2);
        targetPath =  m_config.m_unWritedPath + "/" +rBaseName+"." +QFileInfo(fileName).suffix();
        // targetPath =  m_config.m_unWritedPath + "/" + QFileInfo(fileName).fileName();
        //去掉_r

        break; // 失败待查
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
    QFileInfo fi(dst);
    while (QFile::exists(finalDst)) {
        finalDst = fi.absolutePath() + "/" + fi.baseName() +
                   QString("_%1.").arg(counter++) + fi.completeSuffix();
    }


    if (QFile::rename(src, finalDst)) {
        QFileInfo info(src);
        Tools::log(m_config.m_moduleName, QThread::currentThreadId(),
                   QString("移动文件 %1 : %2 -> %3")
                       .arg(QFileInfo(src).fileName())
                       .arg(QFileInfo(src).absoluteDir().dirName())
                       .arg(QFileInfo(finalDst).absoluteDir().dirName()));
        return true;
    }
    return false;
}
