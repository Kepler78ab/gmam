#include "Tools.h"

Tools::Tools() {}
#include <QString>
//定义表单字段
#include<QDateTime>
#include<QFileInfo>
#include<QDir>
#include<QCoreApplication>
#include<QDebug>

bool Tools::isThreadHandlePrint=true;
bool Tools::isTimePrint=true;
bool Tools::isModulePrint=true;
void Tools::log(QString module, Qt::HANDLE handle, QString msg)
{
    QString strHandle=!isThreadHandlePrint?"":QString("[0x%1]").arg(reinterpret_cast<quintptr>(handle), 0, 16);
    QString strTime=!isTimePrint?"":QString("[%1]").arg(QDateTime::currentDateTime().time().toString());
    QString strModule=!isModulePrint?"":QString("[%1]").arg(module);
    qDebug()<< QString("%1%2%3:%4").arg(strTime).arg(strHandle).arg(strModule).arg(msg);
}
QString Tools::getAbsolutePath(const QString& path) {
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

bool Tools::checkPathExists(const QString& path) {
    QString absolutePath = getAbsolutePath(path);
    return QFileInfo::exists(absolutePath);
}

void Tools::setConfig(bool a, bool b, bool c)
{
    isThreadHandlePrint=a;
    isTimePrint=b;
    isModulePrint=c;
}
