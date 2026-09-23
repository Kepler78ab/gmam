#include "Tools.h"

Tools::Tools() {}
#include <QString>
//定义表单字段
#include<QDateTime>
#include<QFileInfo>
#include<QDir>
#include<QCoreApplication>
#include<QDebug>
bool Tools::isDebug=true;
bool Tools::isThreadHandlePrint=true;
bool Tools::isTimePrint=true;
bool Tools::isModulePrint=true;
void Tools::log(const QString& module, Qt::HANDLE handle, const QString& msg)
{
    // if(!isDebug)
    // {
    //     return;
    // }
    // QString strHandle=!isThreadHandlePrint?"":QString("[0x%1]").arg(reinterpret_cast<quintptr>(handle), 0, 16);
    // QString strTime=!isTimePrint?"":QString("[%1]").arg(QDateTime::currentDateTime().time().toString());
    // QString strModule=!isModulePrint?"":QString("[%1]").arg(module);
    // qDebug()<< QString("%1%2%3:%4").arg(strTime).arg(strHandle).arg(strModule).arg(msg);

    if (!isDebug) return; // 再次保险

    // 优化拼接：使用 QStringBuilder (需要包含 <QStringBuilder>) 或简单的 qPrintable
    // 避免在打印输出时再次产生临时 QString 对象
    QString fullMsg = QString("%1 %2 %3 %4\n")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                          .arg(!isThreadHandlePrint?:reinterpret_cast<quint64>(handle))
                          .arg(!isModulePrint?"":module)
                          .arg(msg);

    QTextStream(stdout) << fullMsg;
// qDebug() << fullMsg;


}
// QString Tools::getAbsolutePath(const QString& path) {
//     QFileInfo fileInfo(path);

//     // 如果已经是绝对路径，直接返回
//     if (fileInfo.isAbsolute()) {
//         return QDir::cleanPath(path);
//     }

//     // 相对路径：拼接应用目录
//     QString basePath = QCoreApplication::applicationDirPath();
//     QString fullPath = QDir(basePath).absoluteFilePath(path);

//     return QDir::cleanPath(fullPath);
// }

// bool Tools::checkPathExists(const QString& path) {
//     QString absolutePath = getAbsolutePath(path);
//     return QFileInfo::exists(absolutePath);
// }

void Tools::setConfig(bool a, bool b, bool c,bool d)
{
    isThreadHandlePrint=a;
    isTimePrint=b;
    isModulePrint=c;
    isDebug=d;
}
