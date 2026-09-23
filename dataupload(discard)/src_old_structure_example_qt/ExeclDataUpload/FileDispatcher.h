#ifndef FILEDISPATCHER_H
#define FILEDISPATCHER_H

#include <QObject>
#include <QString>

class FileDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileDispatcher(QObject *parent = nullptr);

    // 设置各文件夹路径
    void setPaths(const QString &prepare, const QString &writed, const QString &unwrited);
    static void setConfig(const QString &path);


public slots:
    // 核心槽函数：处理移动指令
    // type: 0-正常移动到准备区, 1-写入成功移动到归档区, 2-写入失败移动到未写区
    void slotMoveFile(const QString &srcPath, int type);

    // 数据库状态切换
    void slotDbStatusChanged(bool online);

signals:
    void moveSuccess(const QString &newPath, int type);
    void moveFailed(const QString &filePath, const QString &reason);

private:
    bool performMove(const QString &src, const QString &dst);
    void readJson();

    QString m_preparePath;
    QString m_writedPath;
    QString m_unWritedPath;
    bool m_isDbOnline;
    static QString m_configPath;
};

#endif // FILEDISPATCHER_H
