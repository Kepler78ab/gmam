#include "Applic.h"
#include<QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#include <WidgetStatistics.h>
#include <stdio.h>
#include<QMessageBox>
#endif
Applic::Applic(bool isConsole,const QString &path,QObject *parent)
    : QObject{parent},m_isConsleVisible(isConsole),m_statisticsPath(path)
{
    m_trayIcon = nullptr;
    m_trayMenu = nullptr;
    m_appIcon=QIcon(":/icon/icons/upload.ico");
    m_checkedIcon=QIcon(":/icon/icons/checkedb.ico");
    m_voidIcon=QIcon();
    initTray();
}


Applic::~Applic()
{
    qDebug() << "tray start to destroy";
    disconnect();
    if (m_trayIcon!=nullptr) {
        m_trayIcon->hide();
        delete m_trayIcon;
        m_trayIcon=nullptr;
    }
    if (m_trayMenu!=nullptr) {
        delete m_trayMenu;
        m_trayMenu=nullptr;
    }
    if(m_widgetStatistics!=nullptr)
    {
        delete m_widgetStatistics;
        m_widgetStatistics=nullptr;
    }

    qDebug() << "tray destroyed";
}
void Applic::initTray()
{
    m_trayIcon = new QSystemTrayIcon(this);

    // 请确保项目资源文件中有图标，或者使用系统标准图标进行测试
    m_trayIcon->setIcon(m_appIcon);
    m_trayIcon->setToolTip("控制台程序管理器");
    // 创建菜单
    m_trayMenu = new QMenu();

    m_actionShowLog = m_trayMenu->addAction("输出状态");
    // m_actionSettings = m_trayMenu->addAction("设置参数");
    m_trayMenu->addSeparator();
    m_actionRestart = m_trayMenu->addAction("重启");
    m_actionStatistics=m_trayMenu->addAction("统计界面");
    m_actionQuit = m_trayMenu->addAction("退出");

    // 连接信号
    connect(m_actionShowLog, &QAction::triggered, this, &Applic::onShowLog);
    connect(m_actionStatistics, &QAction::triggered, this, &Applic::onOpenStatistics);
    connect(m_actionRestart, &QAction::triggered, this, &Applic::onRestart);
    connect(m_actionQuit, &QAction::triggered,[this](){
        emit requestQuit();
    });

    m_trayIcon->setContextMenu(m_trayMenu);
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {

        HMENU hMenu = GetSystemMenu(hwnd, FALSE);
        EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if(!m_isConsleVisible)
            ShowWindow(hwnd,SW_HIDE);
        else
            ShowWindow(hwnd,SW_SHOW);
        // qDebug() << "控制台关闭按钮已禁用";
    }
    m_trayIcon->show();
    m_trayIcon->showMessage("启动...", "初始化中");
}
void Applic::onShowLog()
{
    if (!m_isConsleVisible) {
        // 状态：显示中
        m_actionShowLog->setIcon(m_checkedIcon);
    } else {
        // 状态：已隐藏
        m_actionShowLog->setIcon(m_voidIcon);

    }
#ifdef Q_OS_WIN
    // 获取当前控制台窗口句柄
    HWND hWnd = GetConsoleWindow();
    if (hWnd) {
        if (m_isConsleVisible) {
            ShowWindow(hWnd, SW_HIDE); // 隐藏
        } else {
            ShowWindow(hWnd, SW_SHOW); // 显示
            SetForegroundWindow(hWnd); // 提到最前
        }

    }
    m_isConsleVisible = !m_isConsleVisible;
#endif
}
void Applic::onSuccess()
{
    m_trayIcon->showMessage("重启", "已重启");
}
void Applic::slotControllerStatus(bool status)
{
    if(!status)
    {

        m_trayIcon->showMessage("数据库离线", "请检查网络设置");
        m_trayIcon->setToolTip("数据库离线");
        return;
    }
    m_trayIcon->showMessage("数据库上线", "托盘功能已就绪");
    m_trayIcon->setToolTip("数据库在线");

}
void Applic::onRestart()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "重启？", "重启会中断当前所有任务，数据也会丢失",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "准备重启...";
        emit requestRestart(); // 触发你之前写的析构重建逻辑
        return;

    } else {
        qDebug() << "用户取消重启";
        return;
    }
    // emit requestRestart();
}

void Applic::onOpenStatistics()
{
    if(!m_isWidgetOpen)
    {
        m_actionStatistics->setIcon(m_checkedIcon);
        m_isWidgetOpen=true;
        if(m_widgetStatistics)
        {
            m_widgetStatistics->show();
            return;
        }
        m_widgetStatistics=new WidgetStatistics(m_statisticsPath);
        m_widgetStatistics->show();
        return;
    }
    m_actionStatistics->setIcon(QIcon());
    m_isWidgetOpen=false;
    if(m_widgetStatistics!=nullptr)
    {
        m_widgetStatistics->close();
        delete m_widgetStatistics;
        m_widgetStatistics=nullptr;
    }

}
