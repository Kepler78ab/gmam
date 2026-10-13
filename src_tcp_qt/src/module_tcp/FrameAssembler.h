#ifndef MODULE_TCP_FRAMEASSEMBLER_H
#define MODULE_TCP_FRAMEASSEMBLER_H

#include "TcpClientConfig.h"
#include "IFrameMatcher.h"

#include <QByteArray>
#include <QVector>

/**
 * @file FrameAssembler.h
 * @brief 接收缓冲拆帧（P0 Raw / P1 LengthPrefix|PesAscii|Custom）
 */
class FrameAssembler {
public:
    void setConfig(const TcpClientConfig& config);
    void setCustomSplitter(TcpCustomFrameSplitter splitter);
    void clear();

    /** 喂入 socket 数据，返回本轮完整帧 */
    QVector<QByteArray> feed(const QByteArray& chunk);

    /** 最近一次因超长等问题产生的错误文案；空表示无 */
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
