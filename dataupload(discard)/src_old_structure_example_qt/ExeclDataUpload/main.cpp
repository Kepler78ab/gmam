#include "Controller.h"
#include <QCoreApplication>
#include <QObject>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // a.setQuitOnLastWindowClosed(false);
    Controller c;
    return a.exec();
}
