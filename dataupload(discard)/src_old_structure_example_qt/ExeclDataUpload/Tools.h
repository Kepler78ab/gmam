#ifndef TOOLS_H
#define TOOLS_H
#include<QDebug>
class Tools
{
public:
    Tools();
    static void setConfig(bool a,bool b,bool c);
    static void log(QString module, Qt::HANDLE handle, QString msg);
    static QString getAbsolutePath(const QString& path);
    static bool checkPathExists(const QString& path);

private:
    static bool isThreadHandlePrint;
    static bool isTimePrint;
    static bool isModulePrint;
};

#endif // TOOLS_H
