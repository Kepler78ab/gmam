#ifndef TOOLS_H
#define TOOLS_H
#include<QDebug>
class Tools
{
public:
    Tools();
    static void setConfig(bool a,bool b,bool c,bool d);
    static void log(const QString &module, Qt::HANDLE handle, const QString& msg);
    // static QString getAbsolutePath(const QString& path);
    // static bool checkPathExists(const QString& path);

private:
    static bool isDebug;
    static bool isThreadHandlePrint;
    static bool isTimePrint;
    static bool isModulePrint;
};

#endif // TOOLS_H
