#ifndef DATACOLLECTIONANDUPLOAD_H
#define DATACOLLECTIONANDUPLOAD_H
#include <string.h>
#pragma pack(push)
#pragma pack(1)
#include <QString>
//定义表单字段
typedef  struct struTestData
{
    //10-第五试验项目名称-element5 A05
    char m_element[10];
    //10-第五试验项目分析结果值  例如：1.000+4Bytes空白
    char m_resultOfElement[10];
    //10-单位
    char m_unit[20];
    struTestData()
    {
        memset(this,' ',sizeof(struct struTestData));
    }
}STRU_TEST_DATA;
typedef struct struT9PHYTOERP
{
/*
TIMESTAMP          //timeStamp         //时戳
SERIALNO           //serialNo          //序列号
QUEUEID            //queueID           //队列号
HEADER             //header            //头？
DATA               //data              //数据
STATUS             //status            //状态
PROCESSTIME        //processTime       //过程
DESCRIPTION        //description       //描述
*/

    //信息代号：T9PHYTOERP  T9PHY-TO-ERP
    //信息说明：T9物理实验成绩
    //传送方向：T9->ERP
    //传送时机：实验室做完物性试验之后，透过自动采集或人工输入的方式输入到T9系统中，再由T9上传至ERP
    //1-10-传送代号-formId:T9HA:传送拉伸试验实绩到ERP
    char m_formId[10];
    //2-1-状态-inputCode:N:新增 D:删除
    char m_inputCode;
    //3-20-批次-lotNo:生产批号
    char m_lotNo[20];
    //4-10-熔炼号****-HeatNo:13805679(炉号)
    char m_HeatNo[10];
    //5-1-批次试验数量-testNum
    char m_testNum;
    //6-1-批次试验序号-testSeq
    char m_testSeq;
    //7-6-班次-shiftWork
    char m_shiftWork[6];
    //8-2-试验状态-sampStatus
    char m_sampStatus[2];
    //9-20-试片编号****-sampleId
    char m_sampleId[20];
    //10-1-试验种类-testType
    char m_testType;
    //11-10-试验状态、方向位置-couponCode
    char m_couponCode[10];
    //12-2-本次传送试验项目总共个数-number of analysis elements
    char m_numberOfAnalysisElements[2];
    //【11-13REPEAT】n为传送元素个数
    //data
    STRU_TEST_DATA m_data[7];
    char m_end;
    struT9PHYTOERP()
    {
        memset(this,' ',sizeof(struct struT9PHYTOERP));
        m_end = 0;

    }
    void ToData(char **aim)
    {
        memcpy(*aim,this,sizeof(struct struT9PHYTOERP));
    }
}STRU_T9PHY_TO_ERP;

typedef struct struTablePDO
{
    //01-时间驻记-Timestamp-char[17]-yyyyMMddhhmmsszzz
    char m_timeStamp[17];
    //02-时间序号-SeriaNo-int[3]-0 000-999考虑timestamp可能重复，此字段的值与Timestamp结合为复合时间键
    char m_seriaNo[3];
    //03-数据格式-QueueId-char[12]-数据格式名称（根据界面说明书）
    char m_queueId[12];
    //04-标头-Header-char[129]-DI系统所规范的Header（异质系统不许理会此字段）
    char m_header[129];
    //05-数据内容-Data-Varchar[4000]-从异质系统来的数据
    char *m_data;
    //06-状态-Status-char[1]-N:尚未处理 0:处理成功 1：DI Client 回传失败。
    char m_status;
    //07-处理时间-ProcessTime-char[14]-处理成功的日期时间 yyyyMMddhhmmss
    char m_processTime;
    //08-描述-Description-char[1000]-若处理失败，写入失败原因
    char m_description[1000];
    struTablePDO()
    {
        memset(this,' ',sizeof(struct struTablePDO));
    }
}STRU_TABLE_PDO;
typedef struct struFileData
{
    //与中板确认上传数据种类
    //10位的项目号
    QString m_strTestNnumber;
    //A01-屈服强度
    QString m_dA01;
    //A011-下屈服强度
    QString m_dA011;
    //A0113-Rt 规定总延伸强度
    QString m_dA0113;
    //A0114-Rp 规定塑性延伸强度
    QString m_dA0114;


    //A02-抗拉强度f
    QString m_dA02;
    //A03-断后伸长率
    QString m_dA03;

    struFileData()
    {
        m_strTestNnumber = "0";
        m_dA01 = "0";
        m_dA02 = "0";
        m_dA03 = "0";
        m_dA011 = "0";
        m_dA0113 = "0";
        m_dA0114 = "0";
    }


}STRU_FILE_DATA;

#pragma pack(pop)
#endif // DATACOLLECTIONANDUPLOAD_H
