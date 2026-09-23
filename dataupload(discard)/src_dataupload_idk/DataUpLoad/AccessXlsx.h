#ifndef ACCESSXLSX_H
#define ACCESSXLSX_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QThread>
#include <QStringList>
#include <QDir>
#include "DataCollectionAndUpLoad.h"
class AccessXlsx : public QThread
{
    Q_OBJECT
public:
    ~AccessXlsx();

    // 线程主体，负责轮询或监控
    void run() override;

    // 实现单例模式
    static AccessXlsx* GetInstance();

public slots:
    // 监控文件夹变化的槽函数
    void slotDirectoryChanged(const QString &path);
    // 监控文件变化的槽函数
    void slotFileChanged(const QString &path);

private:
    // 构造函数私有化
    explicit AccessXlsx();

    // 模仿 AccessDB 的初始化逻辑
    void Init();

    // 核心功能：读取 Excel 并保存/发送数据
    int SaveData(QString strFileName);

    // 配置读写
    void ReadJson();
    void WriteJson();

private:
    static AccessXlsx *m_instance;

    // 路径配置
    QString m_strSrcFilePath;
    QString m_strDstFilePath;

    // 文件系统监控
    QFileSystemWatcher m_watcher;
    QStringList currEntryList;
    QString m_strFileNameOld;

};

#endif // ACCESSXLSX_H
