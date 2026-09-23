#ifndef FILEDISPATCHER_H
#define FILEDISPATCHER_H

#include <QObject>
#include <QString>
struct DISPATCHER_CONFIG
{
    QString m_moduleName="DISPATCH";
    QString m_preparePath;
    QString m_writedPath;
    QString m_unWritedPath;
    DISPATCHER_CONFIG(){}
};

class FileDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileDispatcher(const DISPATCHER_CONFIG config,QObject *parent = nullptr);
   virtual ~FileDispatcher();
public slots:
    void slotMoveFile(const QString &srcPath, int type);
    // 数据库状态切换
    void slotDbStatusChanged(bool online);
signals:
    void moveSuccess(const QString &newPath, int type);
    void moveFailed(const QString &filePath, const QString &reason);

private:
    bool performMove(const QString &src, const QString &dst);
private:
    DISPATCHER_CONFIG m_config;
    bool m_isDbOnline;
    bool m_isPrint;
    // QString m_preparePath;
    // QString m_writedPath;
    // QString m_unWritedPath;
};

#endif // FILEDISPATCHER_H
