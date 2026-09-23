#ifndef APPLIC_H
#define APPLIC_H
#include "Controller.h"
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include<WidgetStatistics.h>
class Applic : public QObject
{
    Q_OBJECT
public:
    explicit Applic(bool isConsole,const QString & path,QObject *parent = nullptr);
    virtual~Applic();
    void setConsoleStatus(bool a);
    void initTray(); // 初始化托盘
    void onSuccess();
private:
    void openWidget();
signals:
    void requestRestart(); // 发给外部管理者的重启信号
    void requestQuit();
public slots:
    void slotControllerStatus(bool status);
private slots:

    void onShowLog();      // 查看日志
    void onRestart();
    void onOpenStatistics();
private:
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;

    // 动作项
    QAction *m_actionShowLog;
    QAction *m_actionStatistics;
    QAction *m_actionRestart;
    QAction *m_actionQuit;

    bool m_isConsleVisible;
    QString m_statisticsPath;
    bool m_isWidgetOpen=false;
    WidgetStatistics *m_widgetStatistics=nullptr;
    QIcon m_appIcon;
    QIcon m_checkedIcon;
    QIcon m_voidIcon;
};

#endif // APPLIC_H
