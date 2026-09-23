// #include "Controller.h"
// #include "Applic.h"
// #include <QApplication>
// #include <QObject>
// #include <windows.h>
// #include<QFile>
// #include<QDebug>
// #include<QJsonDocument>
// #include<QJsonObject>
// #include<QLockFile>
// #include<QDir>
// #define CONFIG_INI "/./config.ini"
// #include <QSettings>
// #include <QCoreApplication>
// #include <QFileInfo>
// #include "Structure.h"
// #include "container.h"
// bool isAppAutoRun() {
//     QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
//     QString value = reg.value(QApplication::applicationName()).toString();
//     QString currentPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
//     return (value == currentPath);
// }
// bool checkRunning() {
//     // 在临时目录创建一个锁定文件
//     QString lockFilePath = QDir::tempPath() + "/my_app_lock.app.lock";
//     static QLockFile lockFile(lockFilePath);

//     if (!lockFile.tryLock()) {
//         qWarning() << "程序已经在运行中，请勿重复启动！";
//         return true;
//     }
//     return false;
// }
// // int main(int argc, char *argv[])
// // {


// //     QApplication a(argc, argv);
// //     // qDebug()<<isAppAutoRun();
// //     if (checkRunning()) {
// //         return 0; // 如果已运行，直接退出
// //     }

// //     a.setQuitOnLastWindowClosed(false);
// //     // CONSOLE_CONFIG config=readConfig();
// //     // setAutoStart(config.m_autoStart);
// //     Container c;



// //     return a.exec();
// // }
