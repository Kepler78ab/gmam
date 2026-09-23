#ifndef ORACLEDB_H
#define ORACLEDB_H

#include <QObject>
#include <QLibrary>
#include <QSqlDatabase>
#include <QDateTime>

class OracleDB : public QObject
{
    Q_OBJECT
public:
    explicit OracleDB(QObject *parent = nullptr);
    ~OracleDB();
    bool GetDBIsOpen();
    static OracleDB *GetInstance();
    int WriteData(char *data,int len);
    void Select();

signals:

private slots:
    void slotReConnectTimeOut();

private:
    void Init();
    void ReadJson();
    void WriteJson();
    bool ReConnectToNet();
    bool ReConnectToDB();


    QLibrary *m_ptrOciLib;
    QLibrary *m_ptrOciLib1;
    QSqlDatabase m_DB;
    QSqlDatabase m_DB1;

    QString m_strHostName;
    int m_iPort;
    QString m_strDatabaseName;
    QString m_strUserName;
    QString m_strPassword;
    QString m_strTableName;
    static OracleDB* m_instance;
    QString m_strDBConnectionName;
    static int m_iConnectionNum;
private:

    QDateTime m_lastMinDateTime;
    bool m_bCheckConnectDB;
    bool m_bCheckConnectNet;

};

#endif // ORACLEDB_H
