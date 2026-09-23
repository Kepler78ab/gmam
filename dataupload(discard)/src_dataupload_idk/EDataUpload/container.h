#ifndef CONTAINER_H
#define CONTAINER_H

#include <QObject>
#include "Applic.h"
#include "Controller.h"
class Container : public QObject
{
    Q_OBJECT
public:
    explicit Container(QObject *parent = nullptr);
    void setAutoStart(bool enable);
    virtual ~Container();
signals:
    void restart();
private slots:
    void slotOnQuit();
    void slotOnRestart();
private:
    QString getAbsolutePath(const QString& path);
    bool checkPathExists(const QString& path);
    bool init();
    bool readConfig();
    bool readControllerConfig();
private:
    Applic *m_app=nullptr;
    Controller *m_c=nullptr;
    CONSOLE_CONFIG m_config;
    CONTROLLER_CONFIG m_controllerConfig;
    QString m_statisticsPath;
};

#endif // CONTAINER_H
