#ifndef FILEMONITER_H
#define FILEMONITER_H

#include <QObject>
#include <QThread>
#include<QFileSystemWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include<QWaitCondition>
class FileMoniter : public QThread
{
    Q_OBJECT
public:
    ~FileMoniter();
    static FileMoniter*instance();
    static void setConfig(QString filePath);
    void run() override;
    void stop();
private:
    explicit FileMoniter();
    void readJson();
    QStringList scanFiles(const QString& folderPath);
    QStringList filterValidFiles(const QStringList& allFiles) const;
    bool processSingleFile(const QString &fileName);
    void processAllNewFile(const QString&part);
    void cleanupProcessRecord();
    bool isFileReady(const QString& filePath) const;
signals:
    void getData(QString FileName);
    void immediateProcess();
    void requestMove(QString fullPath,int type);
private slots:
    void slotDirectoryChanged(const QString &path);
public slots:
    void slotRemoveFromProcessed(const QString &filename);
private:
    static FileMoniter *m_instance;
    static QString m_configPath;
    QString m_unScanFolderPath;
    QString m_unWritedFolderPath;
    QString m_writedFolderPath;
    QString m_prepareFolderPath;

    QFileSystemWatcher m_watcher;
    QStringList currEntryList;
    QString m_strFileNameOld;

    QString m_fileForm;
    QString m_filter;
    QString m_filterRegEx;

    int m_scanInterval=10;
    int m_waitInterval=5;

    QMutex m_mutex;
    QMutex m_processMutex;
    QSet<QString> m_processedFiles;


};

#endif // FILEMONITER_H
