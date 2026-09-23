#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H
#include "DataCollectionAndUpLoad.h"
#include <QObject>
#include <QMap>
//完成文件获取，字段拆解
class DataCollection : QObject
{
    Q_OBJECT
public:
    DataCollection();
    ~DataCollection();
    int SendData(STRU_FILE_DATA fileData);
    int SendData1(STRU_FILE_DATA fileData);
private:
    void ReadFile(QString strFileName);
    QString GetSteelType(QString strLotNum);
    void ReadSteelConfig();
    void WriteSteelConfig();

    bool GetConfigBySteelType(QString strSteelType);

    QMap<QString,bool> m_mapSteelTypeAndValue;

};

#endif // DATACOLLECTION_H
