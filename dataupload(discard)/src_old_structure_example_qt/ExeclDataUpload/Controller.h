#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "FileMoniter.h"
#include "XlsxReader.h"
#include "OracleDB.h"
#include "FileDispatcher.h"
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    void readConfig();
    void init();
private:
    QString m_dbConfig;
    QString m_readFileConfig;
    QString m_fileMointerConfig;
    bool isThreadHandlePrint;
    bool isTimePrint;
    bool isModulePrint;


    bool isDBInit;
    bool isXlsxInit;
    bool isFileMoniterInit;

    QThread *m_xlsxThread;
    QThread *m_dbThread;
    QThread *m_dispatcherThread;

    FileMoniter *m_fileMoniterThread;

    ExcelOperator *m_xlsxReader;

    OracleDB *m_db;
    FileDispatcher *m_dispatcher;

    QString m_prepareFolderPath;
    QString m_unWritedFolderPath;
    QString m_writedFolderPath;


signals:
};

#endif // CONTROLLER_H
