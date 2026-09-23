#ifndef FILEMONITER_H
#define FILEMONITER_H

#include <QObject>
#include <QThread>
#include<QFileSystemWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include<QWaitCondition>
#include<QTimer>
struct MONITER_CONFIG
{
    QString m_moduleName="MONITER";
    QString m_unScanFolderPath;
    QString m_unWritedFolderPath;
    int m_scanInterval;
    int m_waitInterval;
    QString m_filter;
    QString m_fileForm;
    QString m_filterRegEx;
    MONITER_CONFIG()
        : m_scanInterval(10)
        , m_waitInterval(5)
    {}

};

class FileMoniter : public QObject
{
    Q_OBJECT
public:
   virtual ~FileMoniter();
    explicit FileMoniter(const MONITER_CONFIG& config,QObject *parent=nullptr);
    void init();
private:
    QStringList scanFiles(const QString& folderPath);
    QStringList filterValidFiles(const QStringList& allFiles) const;
    bool processSingleFile(const QString &fileName);
    void processAllNewFile(const QString&part);
    void cleanupProcessRecord();
    bool isFileReady(const QString& filePath) const;
private slots:
    void slotAutoScan();
    void slotPassiveScan();
signals:
    void getData(const QString& FileName);
    void immediateProcess();
    void requestMove(const QString& fullPath,int type);
private slots:
    void slotDirectoryChanged(const QString &path);
public slots:
    void slotRemoveFromProcessed(const QString &filename);
    void slotDbStatusChanged(bool isDBonline);
    void slotStart();
    void slotStop();

private:
    QFileSystemWatcher m_watcher;
    QStringList currEntryList;

    MONITER_CONFIG m_config;
    QMutex m_processMutex;
    QSet<QString> m_processedFiles;
    bool m_isDbOnline=true;
    bool m_isMoniterRun=true;
    QTimer *m_scanTimer;

};

#endif // FILEMONITER_H
