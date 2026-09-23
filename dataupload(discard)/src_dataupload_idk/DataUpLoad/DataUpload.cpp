#include "DataUpload.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QJsonParseError>
//等待数字智联刘生建提供中冠接口说明
DataUpload::DataUpload()
{

}
DataUpload::~DataUpload()
{

}

void DataUpload::SendData(STRU_T9PHY_TO_ERP data)
{

    QNetworkAccessManager manager;
    QUrl url("http://your-java-server-address/api");
    QJsonObject jsonObject;
    jsonObject["key"] = "value"; // 根据需要添加键值对

    QJsonDocument jsonDoc(jsonObject);
    QByteArray jsonBytes = jsonDoc.toJson();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager.post(request, jsonBytes);

    // 连接信号槽以处理响应
    connect(reply, &QNetworkReply::finished,
            [&]()
        {
            if (reply->error() == QNetworkReply::NoError)
            {
                QByteArray responseData = reply->readAll();
                QJsonParseError parseError;
                QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);
                if (parseError.error == QJsonParseError::NoError)
                {
                    if (responseDoc.isObject())
                    {
                        QJsonObject responseObject = responseDoc.object();
                        // 处理Java服务器返回的JSON数据
                    }
                }
            }
            else
            {
                // 处理错误
            }

            reply->deleteLater();
        }
    );
}
