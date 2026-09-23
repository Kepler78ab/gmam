#ifndef ACCESSDB_H
#define ACCESSDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileSystemWatcher>
#include <QThread>

class AccessDB : public QThread
{
    Q_OBJECT
public:
    ~AccessDB();
    void run() override;
    static AccessDB* GetInstance();

signals:

public slots:
    void slotDirectoryChanged(const QString &path);
    void slotFileChanged(const QString &path);

private:
    explicit AccessDB();

    void Init();
    int SaveData(QString strFileName);
    int SaveData1(QString strFileName);
    void ReadJson();
    void WriteJson();

private:
    static AccessDB *m_instance;

    QString m_strSrcFilePath;
    QString m_strDstFilePath;

    QFileSystemWatcher m_watcher;
    QStringList currEntryList;
    QString m_strFileNameOld;


};

#endif // ACCESSDB_H
