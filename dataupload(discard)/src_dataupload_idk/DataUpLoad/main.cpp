#include <QCoreApplication>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QSqlDriver>
#include "AccessDB.h"
#include "OracleDb.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QStringList strlist = QSqlDatabase::drivers();
    foreach(QString strdriver,strlist)
    {
        qDebug()<<strdriver;
    }

    AccessDB *dbAccess = AccessDB::GetInstance();
    dbAccess->start();

    qDebug()<<"this is main thread"<<QThread::currentThread();
    // OracleDB::GetInstance()->GetDBIsOpen();

    qDebug()<<"Start~!!!!!";
    return a.exec();
}
