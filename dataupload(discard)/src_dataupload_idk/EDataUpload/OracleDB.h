#ifndef ORACLEDB_H
#define ORACLEDB_H

#include <QObject>
#include <QLibrary>
#include <QSqlDatabase>
#include <QDateTime>
#include <QTimer>
struct DB_CONFIG{
    QString m_moduleName="DB";
    QString m_strHostName="";//主机地址
    int m_iPort=0;//端口
    QString m_strDatabaseName="";//数据库名
    QString m_strUserName="";//账号
    QString m_strPassword="";//密码
    QString m_strTableName="";//表名
    int m_reConnectInterval=5;//重连时间
    int m_serialNo;//时间戳后面什么玩意的
    QString m_queueId="";//标识符
    QString m_desc="";//描述 默认为空
    DB_CONFIG(){};
};

class OracleDB : public QObject
{
    Q_OBJECT
public:
    explicit OracleDB(QObject *parent = nullptr);
    virtual~OracleDB();
    bool GetDBIsOpen();
    static void setConfig(const QString& filePath);
public slots:
    void slotInit();
signals:
    void dataNotWrited(const QString& filename);
    void dataWrited(const QString& filename);
    void dbStatusSignal(bool status);

private slots:
    void slotReConnectTimeOut();
    void slotRecvData(const QByteArray& data,const QString& filename);

private:
    int WriteData(const QByteArray &data,const QString& filename);
    void Init();
    void ReadJson();
    // void WriteJson();
    bool ReConnectToNet();
    bool ReConnectToDB();

    QSqlDatabase m_DB;


private:

    // QDateTime m_lastMinDateTime;
    static QString m_configPath;
    static int m_iConnectionNum;
    QString m_strDBConnectionName;


    QTimer *m_timerReConnect;
    bool m_isNetOnline;
    bool m_isDBOnline;



    //config
    DB_CONFIG m_config;
};

#endif // ORACLEDB_H
