#include "AccessDB.h"
#include <QSqlRecord>
#include <QDebug>
#include <QtWidgets/QFileDialog>
#include "DataCollectionAndUpLoad.h"
#include "DataCollection.h"
#define FILE_PATH_CONFIG_INI "./ReadFileConfig.ini"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <windows.h>
#include <QRegExp>

/*
 * 批号:{BatchNumber},
 * 钢种:{SteelType},
 * 试样宽度:{BO},
 * 试样厚度:{AO},
 * 上屈服强度:{ReH},
 * 下屈服强度:{ReL},
 * 抗拉强度:{RM},
 * 伸长率:{A},
 * 实验时间：{Time}
*/
AccessDB *AccessDB::m_instance = NULL;
AccessDB::AccessDB()
    : QThread()
{
     qDebug()<<"AccessDb create in"<<QThread::currentThread();
    WriteJson();
    Init();
    ReadJson();
   // QString strFilePath = m_strSrcFilePath.replace(" ","\' \'");
    return ;

    currEntryList.clear();

    const QDir dir(m_strSrcFilePath);

    currEntryList = dir.entryList(QDir::NoDotAndDotDot|QDir::Files);

    bool ret = m_watcher.addPath(m_strSrcFilePath);
    ret = connect(&m_watcher,SIGNAL(directoryChanged(QString)),this,SLOT(slotDirectoryChanged(QString )));
    ret = connect(&m_watcher,SIGNAL(fileChanged(QString)),this,SLOT(slotFileChanged(QString )));

    // connect(this,SIGNAL(verifyThreadRight),this,SLOT(slotVerifyThread));



    return ;

    //文件转移法
    QDir floder(m_strSrcFilePath);
    QStringList fileNames = floder.entryList(QDir::Files);
    foreach (QString filename, fileNames)
    {
        //文件过滤
        //仅处理 .mdb 文件
        if(!filename.contains(".mdb"))
            continue;
        //文件名称开头数字过滤
        QFileInfo fileInfo(filename);
        QString strBaseName = fileInfo.baseName();
       /* if(strBaseName.at(0)!='1')
            continue ;*/
        //文件长度过滤
        if(strBaseName.length()!=10)
            continue;

        int ret = SaveData(m_strSrcFilePath + '/' + filename);
        qDebug()<<"SaveData ret = "<<ret<<"ppp";

        if(ret==0)//ret
        {
            //处理成功，做文件转移
            QFile file(m_strSrcFilePath + '/' + filename);
            QDir dir;
            int counter = 1;
            QFileInfo fileInfo(filename);
            //做文件名称区分
            while(dir.exists(m_strDstFilePath+ '/' + filename))
            {
                filename = fileInfo.baseName()+"_"+QString::number(counter)+ fileInfo.fileName().remove(fileInfo.baseName());
                counter++;
            }
            if(file.rename(m_strDstFilePath + '/' + filename))
            {
                qDebug()<<"File:"<<m_strDstFilePath + '/' + filename<<" moved successfully";
            }
            else
            {
                qDebug()<<"Failed to move the file:"<<m_strDstFilePath + '/' + filename;
            }
        }
    }
}

AccessDB::~AccessDB()
{

}
void AccessDB::run()
{
    qDebug()<<"AccessDB thread move2"<<QThread::currentThread();
    // static OracleDB ora;
    const QDir dir(m_strSrcFilePath);
    qDebug()<<"this work in"<<QThread::currentThread();
    DataCollection dc;
    m_strFileNameOld = "";
    //获取文件夹下最新文件列表
    QStringList newEntryList = dir.entryList(QDir::NoDotAndDotDot|QDir::Files,QDir::Time);
    if(newEntryList.size()>0)
    {
        //获取文件夹下最新采集文件列表
        QStringList newEntryList1 = newEntryList.filter(".xlsx");
        if(newEntryList1.size()>0)
        {
            //10月宽厚板改动---12位号
            //获取10位名称，卷板精轧采集文件--^.{3,20}$
            //卷板精轧
            //QRegExp re("^.{13,14}$");
            //"^(?:.{5}|.{10})$"  # 单行文本（不包含换行符）
            //宽厚板
            //QRegExp re("^.{19,19}$");
            QRegExp re("^(.{14,14}$)|(.{16,16}$)");
            QStringList newEntryList2 = newEntryList1.filter(re);
            if(newEntryList2.size()>0)
            {
                m_strFileNameOld = newEntryList2.first();
            }
        }
    }
    while(1)
    {
        //休眠120秒=2分钟---2分钟太慢了
        sleep(10);
        qDebug()<<"AccessDB thread run in"<<QThread::currentThread();
        const QDir dir(m_strSrcFilePath);

        //获取文件夹下最新文件列表
        QStringList newEntryList = dir.entryList(QDir::NoDotAndDotDot|QDir::Files,QDir::Time);
        // qDebug()<<m_strSrcFilePath<<"file count="<<newEntryList.size();
        if(newEntryList.size()<1)
            continue;
        //获取文件夹下最新采集文件列表
        QStringList newEntryList1 = newEntryList.filter(".xlsx");
        // qDebug()<<m_strSrcFilePath<<".mdb file count="<<newEntryList1.size();
        if(newEntryList1.size()<1)
            continue;
        //获取10位名称，卷板精轧采集文件--^.{3,20}$
        //卷板精轧
        QRegExp re("^(.{14,14}$)|(.{16,16}$)");
        //宽厚板
        //QRegExp re("^.{19,19}$");
        QStringList newEntryList2 = newEntryList1.filter(re);
        if(newEntryList2.size()<1)
            continue;

        QString strFirstFileName = newEntryList2.first();
        if(m_strFileNameOld==strFirstFileName)
        {
            //已处理成功
            qDebug()<<"选后也一样啊";
            continue;
        }
        qDebug()<<"睡30s";
        sleep(30);
        QString fileName  = m_strSrcFilePath+"/"+strFirstFileName;
        qDebug()<<"准备把"<<fileName<<"发了";
        int ret = SaveData1(fileName);
        if(0==ret)
        {
            // m_watcher.removePath(filePath);
            qDebug()<<"FileHandle success: "<< fileName;

            m_strFileNameOld = strFirstFileName;
        }
        else
        {
            if(-2==ret)
            {
                //未处理的数据--空数据文件 -2：空数据 -3缺少断后伸长率 -4缺少抗拉强度
                //errorFiles += filename;
            }

            qDebug()<<"FileHandle failed: "<<ret<<fileName;
        }
    }
}

AccessDB* AccessDB::GetInstance()
{
    if(NULL == m_instance)
    {
        m_instance = new AccessDB();
    }

    return m_instance;
}


void AccessDB::slotDirectoryChanged(const QString &path)
{
    //等待40s，等待现场师傅操作
    //Sleep(30000);

    //文件夹更新，添加所有复合文件到监视列表中
    const QDir dir(path);

    QStringList newEntryList = dir.entryList(QDir::NoDotAndDotDot|QDir::Files,QDir::Time);
    if(newEntryList.size()<1)
        return;
    //获取文件夹下最新采集文件列表
    QStringList newEntryList1 = newEntryList.filter(".mdb");
    if(newEntryList1.size()<1)
        return;
    //获取10位名称，卷板精轧采集文件--^.{3,20}$
    QRegExp re("^.{12,12}$");
    QStringList newEntryList2 = newEntryList1.filter(re);
    if(newEntryList2.size()<1)
        return;

    //清空监视
    if(m_watcher.files().size()>0)
    {
        m_watcher.removePaths(m_watcher.files());
    }
    //添加新的监视
    for(int i = 0;i<newEntryList2.size();i++)
    {
        m_watcher.addPath(m_strSrcFilePath+"/"+newEntryList2.at(i));
    }

   // int a = 0;
}
void AccessDB::slotFileChanged(const QString &path)
{
    //文件改动法
    qDebug()<<"Re HandleFilePath"<<path<<"-----";

    int ret = SaveData(path);
    qDebug()<<"ret :"<<ret;
    if(0==ret)
    {
        //取消该文件监控
        //m_watcher.removePath(path);
        qDebug()<<"Re FileHandle success: "<< path;
    }
    else
    {

        qDebug()<<"Re FileHandle failed: "<<ret<<path;
    }
}


void AccessDB::Init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");//,"Access_Connect"
    QString strFilePath = QString("E:/dev/QtCreatorProject/DataUpLoadpro/Bin/File1/22520808.mdb");
    QString strDatabaseName = QString("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=%1;uid=0;pwd=1;").arg(strFilePath);
    db.setDatabaseName(strDatabaseName);
    if(!db.open())
    {
        printf("--------------------Open AccessDB failed \n");
        QSqlError err = db.lastError();
        qDebug()<<err.text();
        return;
    }
    QString strSelect = QString("select top 1 Steel_grade,SLABNO from HIS_Plan_data where LOTNUM = '2330801'");
    QSqlQuery query;
    QString strSQL = QString("select * from ParamFactValue");

    if(!query.exec(strSQL))
    {
        printf("select failed \n");
        QSqlError err = query.lastError();
        qDebug()<<err.text();
        return ;
    }
    QStringList strList;
    QSqlRecord sqlRecord(query.record());
    for(int i =0;i<sqlRecord.count();i++)
    {
        strList<<sqlRecord.fieldName(i);

    }
    if(strList.size()<5)return ;

    while(query.next())
    {
        for(int i =0;i<strList.size();i++)
        {
            qDebug()<<strList.at(i)+"--------"<<query.value(strList.at(i));
        }
        printf("\n");
    }
    db.close();
}
int AccessDB::SaveData(QString strFileName)
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))//
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        db = QSqlDatabase::addDatabase("QODBC");//,"QODBC_Connection"
    }
    QString strFile = strFileName;//.replace(' ',"\' \'");
    QString strTest = QString::fromLocal8Bit("DBQ=%1").arg(strFile);//FIL={MS Access Database};
    QString strDatabaseName = QString::fromLocal8Bit("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%1;").arg(strFile);//FIL={MS Access}; uid=0;pwd=1;
    db.setDatabaseName(strDatabaseName);
    //db.transaction();
    if(!db.open())//||db.lastError().type()!=QSqlError::NoError
    {
        //bool ret = db.isOpen();
        QString str = db.driverName();
        //bool ret1 = db.isOpenError();
        printf("--------------------Open AccessDB failed \n");
        qDebug()<<"--------------------FileName:"<<strFileName<<endl;
        QSqlError err = db.lastError();
        qDebug()<<"["<<strDatabaseName<<"]";
        qDebug()<<"--------------------"<<err.text();
        return -1;
    }
    QSqlQuery query;
    QString strSQL = QString("select * from ParamFactValue");

    if(!query.exec(strSQL))
    {
        printf("--------------------select ParamFactValue failed \n");
        QSqlError err = query.lastError();
        qDebug()<<"--------------------"<<err.text();
        db.close();
        return  -1;
    }
    QStringList strList;
    QSqlRecord sqlRecord(query.record());
    for(int i =0;i<sqlRecord.count();i++)
    {
        strList<<sqlRecord.fieldName(i);

    }
    if(!strList.contains("Name"))
    {
        printf("--------------------less 'Name'\n");
        db.close();
        return  -1;
    }
    if(!strList.contains("TheValue"))
    {
        printf("--------------------less 'TheValue'\n");
        db.close();
        return  -1;
    }
    if(!strList.contains("Unit"))
    {
        printf("--------------------less 'Unit'\n");
        db.close();
        return  -1;
    }
    QString strName;
    QString strTheValue;
    QString strUnit;
    STRU_FILE_DATA data;
    while(query.next())
    {
        /*打印全部数据-AccessDB*/
        for(int i =0;i<strList.size();i++)
        {
            qDebug()<<strList.at(i)+"--------------------"<<query.value(strList.at(i));
        }
        strName = query.value("Name").toString();
        if(strName.contains(QString::fromLocal8Bit("试验项目号")))
        {
            data.m_strTestNnumber = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("上屈服强度")))
        {
            data.m_dA01 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("下屈服强度")))
        {
            //data.m_dA011 = query.value("TheValue").toString();
            float a = query.value("TheValue").toFloat();
           // a = a+1;
            data.m_dA011 = QString::number(a);
        }
        else if(strName.contains(QString::fromLocal8Bit("抗拉强度")))
        {
            data.m_dA02 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("断后伸长率")))
        {
            data.m_dA03 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("规定总延伸强度")))
        {
            data.m_dA0113 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("规定塑性延伸强度")))
        {
            data.m_dA0114 = query.value("TheValue").toString();
        }
        else
        {

        }
    }
    db.close();
    if(data.m_strTestNnumber.isEmpty()||data.m_strTestNnumber=="0")
        return -2;
    if(data.m_dA03.isEmpty()||data.m_dA03=="0")
        return -3;
    if(data.m_dA02.isEmpty()||data.m_dA02=="0")
        return -4;
    DataCollection dataCollection;

    return dataCollection.SendData(data);
}
int AccessDB::SaveData1(QString strFileName)
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))//
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        db = QSqlDatabase::addDatabase("QODBC");//,"QODBC_Connection"
    }
    QString strFile = strFileName;//.replace(" ","\" \"");//;

    QString strDatabaseName = QString::fromLocal8Bit("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=[%1]").arg(strFile);//FIL={MS Access}; uid=0;pwd=1;
    db.setDatabaseName(strDatabaseName);
    if(!db.open())//||db.lastError().type()!=QSqlError::NoError
    {
        //bool ret = db.isOpen();
        QString str = db.driverName();
        //bool ret1 = db.isOpenError();
        printf("--------------------Open AccessDB failed \n");
        qDebug()<<"--------------------FileName:"<<strFileName<<endl;
        QSqlError err = db.lastError();
        qDebug()<<"["<<strDatabaseName<<"]";
        qDebug()<<"--------------------"<<err.text();
        return -1;
    }
    QSqlQuery query;
    QString strSQL = QString("select * from ParamFactValue");

    if(!query.exec(strSQL))
    {
        printf("--------------------select ParamFactValue failed \n");
        QSqlError err = query.lastError();
        qDebug()<<"--------------------"<<err.text();
        db.close();
        return  -1;
    }
    QStringList strList;
    QSqlRecord sqlRecord(query.record());
    for(int i =0;i<sqlRecord.count();i++)
    {
        strList<<sqlRecord.fieldName(i);

    }
    if(!strList.contains("Name"))
    {
        printf("--------------------less 'Name'\n");
        db.close();
        return  -1;
    }
    if(!strList.contains("TheValue"))
    {
        printf("--------------------less 'TheValue'\n");
        db.close();
        return  -1;
    }
    if(!strList.contains("Unit"))
    {
        printf("--------------------less 'Unit'\n");
        db.close();
        return  -1;
    }
    QString strName;
    QString strTheValue;
    QString strUnit;
    STRU_FILE_DATA data;
    while(query.next())
    {
        /*打印全部数据-AccessDB*/
        for(int i =0;i<strList.size();i++)
        {
            qDebug()<<strList.at(i)+"--------------------"<<query.value(strList.at(i));
        }
        strName = query.value("Name").toString();
        if(strName.contains(QString::fromLocal8Bit("试验项目号")))
        {
            data.m_strTestNnumber = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("上屈服强度")))
        {
            data.m_dA01 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("下屈服强度")))
        {
            //data.m_dA011 = query.value("TheValue").toString();
            float a = query.value("TheValue").toFloat();
            // a = a+1;
            data.m_dA011 = QString::number(a);
        }
        else if(strName.contains(QString::fromLocal8Bit("抗拉强度")))
        {
            data.m_dA02 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("断后伸长率")))
        {
            data.m_dA03 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("规定总延伸强度")))
        {
            data.m_dA0113 = query.value("TheValue").toString();
        }
        else if(strName.contains(QString::fromLocal8Bit("规定塑性延伸强度")))
        {
            data.m_dA0114 = query.value("TheValue").toString();
        }
        else
        {

        }
    }
    db.close();
    if(data.m_strTestNnumber.isEmpty()||data.m_strTestNnumber=="0")
        return -2;
    if(data.m_dA03.isEmpty()||data.m_dA03=="0")
        return -3;
    if(data.m_dA02.isEmpty()||data.m_dA02=="0")
        return -4;
    DataCollection dataCollection;

    return dataCollection.SendData1(data);
}

void AccessDB::ReadJson()
{
    QString strFileName = QCoreApplication::applicationDirPath() + FILE_PATH_CONFIG_INI;
    QFile file(strFileName);
    if(!file.open((QIODevice::ReadOnly)))
    {
        qDebug()<<"open file:"<<strFileName<<"failed!";
        return ;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if(jsonDoc.isObject())
    {
        QJsonObject jsonObject = jsonDoc.object();
        if(!jsonObject.value("SrcFilePath").isNull())
            m_strSrcFilePath = jsonObject.value("SrcFilePath").toString();
        if(!jsonObject.value("DstFilePath").isNull())
            m_strDstFilePath = jsonObject.value("DstFilePath").toString();
    }

}
void AccessDB::WriteJson()
{
    //不写只读
    QJsonObject object;
    QString srcFilePath("E:/dev/QtCreatorProject/DataUpLoadpro/Bin/File1");
    QString dstFilePath("E:/dev/QtCreatorProject/DataUpLoadpro/Bin/File 1 2");
    object.insert("SrcFilePath",srcFilePath);
    object.insert("DstFilePath",dstFilePath);
    QJsonDocument jsonDoc(object);
    QString strFileName = QCoreApplication::applicationDirPath() + FILE_PATH_CONFIG_INI;
    QFile file(strFileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"open file:"<<strFileName<<"failed";
        return ;
    }
    file.write(jsonDoc.toJson());
    file.close();
}






























