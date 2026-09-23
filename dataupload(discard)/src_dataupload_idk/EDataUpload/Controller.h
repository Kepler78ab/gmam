#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "FileMoniter.h"
#include "XlsxReader.h"
#include "OracleDB.h"
#include "FileDispatcher.h"
struct CONTROLLER_CONFIG
{
    QString m_dbConfig;
    bool isThreadHandlePrint;
    bool isTimePrint;
    bool isModulePrint;
    bool isDebug;
    bool isControllerOnline=false;
    bool isDBInit;
    bool isXlsxInit;
    bool isFileMoniterInit;
    MONITER_CONFIG m_moniterConfig;
    XLSXR_CONFIG m_xlsxConfig;
    DISPATCHER_CONFIG m_dispatcherConfig;
    CONTROLLER_CONFIG() {}
};
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(CONTROLLER_CONFIG config,QObject *parent = nullptr);
   virtual ~Controller();
private:
    // bool readConfig();
    bool init();
    QString getAbsolutePath(const QString& path);
    bool checkPathExists(const QString& path);
signals:
    void controllerStatus(bool status);
private:

    QThread *m_xlsxThread;
    QThread *m_dbThread;
    QThread *m_dispatcherThread;
    QThread *m_mointerThread;

    ExcelOperator *m_xlsxReader;
    OracleDB *m_db;
    FileDispatcher *m_dispatcher;
    FileMoniter *m_moniter ;

    CONTROLLER_CONFIG m_config;

};

#endif // CONTROLLER_H
