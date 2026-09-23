#ifndef ORACLEDB_H
#define ORACLEDB_H

#include <QObject>
#include <QLibrary>
#include <QSqlDatabase>
#include <QDateTime>
#include <QTimer>
class OracleDB : public QObject
{
    Q_OBJECT
public:
    explicit OracleDB(QObject *parent = nullptr);
    ~OracleDB();
    bool GetDBIsOpen();
    static OracleDB *GetInstance();
    static void setConfig(const QString& filePath);
    int WriteData(char *data,int len);
    int WriteData(QByteArray data,const QString& filename);
    void Select();
    void moveToThread(QThread *targetThread);
    void startReconnectTimer();

signals:
    void dataNotWrited(const QString& filename);
    void dataWrited(const QString& filename);
    void dbStatusSignal(bool status);

private slots:
    void slotReConnectTimeOut();
    void slotRecvData(QByteArray data,const QString& filename);

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

    QTimer *m_timerReConnect;
private:

    QDateTime m_lastMinDateTime;
    bool m_bCheckConnectDB;
    bool m_bCheckConnectNet;

    static QString m_configPath;
    int m_serialNo;
    QString m_queueId;
    QString m_desc;
};

#endif // ORACLEDB_H
