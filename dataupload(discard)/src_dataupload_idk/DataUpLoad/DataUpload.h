#ifndef DATAUPLOAD_H
#define DATAUPLOAD_H
#include "DataCollectionAndUpLoad.h"
#include <QObject>
//完成字段填表，服务上传
class DataUpload : QObject
{
    Q_OBJECT
public:
    DataUpload();
    ~DataUpload();
    void SendData(STRU_T9PHY_TO_ERP data);//URL,发送数据----数字智联提供数据库访问数表，暂时舍弃
    void SendData();
};

#endif // DATAUPLOAD_H
