
#include <QCoreApplication>
#include <QDebug>
#include "GmamSelfTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!gmamRunAllTests()) {
        qWarning() << "GMAM tests FAILED";
        return 1;
    }
    qInfo() << "GMAM tests ALL PASSED";
    return 0;
}
