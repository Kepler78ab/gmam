#ifndef MODULE_TCP_FRAMEASSEMBLER_H
#define MODULE_TCP_FRAMEASSEMBLER_H

#include "TcpClientConfig.h"
#include "IFrameMatcher.h"

#include <QByteArray>
#include <QVector>

/**
 * @file FrameAssembler.h
 * @brief 接收侧拆帧：Raw / LengthPrefix / PesAscii / Custom。
 */
class FrameAssembler {
public:
    /**
     * @brief 应用拆帧相关配置（frameMode、长度前缀等）。
     * @param config 客户端配置
     */
    void setConfig(const TcpClientConfig& config);

    /**
     * @brief 设置 Custom 模式下的拆帧回调。
     * @param splitter 自定义切帧函数
     */
    void setCustomSplitter(TcpCustomFrameSplitter splitter);

    /**
     * @brief 清空内部缓冲与最近错误。
     */
    void clear();

    /**
     * @brief 喂入本轮 socket 数据并尽量拆出完整帧。
     * @param chunk 原始字节块
     * @return 本轮完整帧列表（可空）
     */
    QVector<QByteArray> feed(const QByteArray& chunk);

    /**
     * @brief 取出并清除最近一次拆帧错误文案。
     * @return 错误说明；空表示无
     */
    QString takeLastError();

private:
    QVector<QByteArray> feedRaw(const QByteArray& chunk);
    QVector<QByteArray> feedLengthPrefix();
    int parseLengthField(const QByteArray& buf, int* ok) const;

    TcpClientConfig m_config;
    TcpCustomFrameSplitter m_custom;
    QByteArray m_buffer;
    QString m_lastError;
};

#endif // MODULE_TCP_FRAMEASSEMBLER_H
