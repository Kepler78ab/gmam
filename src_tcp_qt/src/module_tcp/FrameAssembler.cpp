#include "FrameAssembler.h"

void FrameAssembler::setConfig(const TcpClientConfig& config)
{
    m_config = config;
}

void FrameAssembler::setCustomSplitter(TcpCustomFrameSplitter splitter)
{
    m_custom = std::move(splitter);
}

void FrameAssembler::clear()
{
    m_buffer.clear();
    m_lastError.clear();
}

QString FrameAssembler::takeLastError()
{
    const QString e = m_lastError;
    m_lastError.clear();
    return e;
}

QVector<QByteArray> FrameAssembler::feed(const QByteArray& chunk)
{
    if (m_config.frameMode == TcpFrameMode::Raw)
        return feedRaw(chunk);

    m_buffer.append(chunk);

    if (m_config.frameMode == TcpFrameMode::Custom) {
        if (!m_custom)
            return {};
        return m_custom(m_buffer);
    }

    // LengthPrefix / PesAscii
    return feedLengthPrefix();
}

QVector<QByteArray> FrameAssembler::feedRaw(const QByteArray& chunk)
{
    QVector<QByteArray> out;
    if (!chunk.isEmpty())
        out.push_back(chunk);
    return out;
}

int FrameAssembler::parseLengthField(const QByteArray& buf, int* ok) const
{
    *ok = 0;
    const int off = m_config.lengthPrefixOffset;
    const int sz = m_config.lengthPrefixSize;
    if (off < 0 || sz <= 0 || buf.size() < off + sz)
        return -1;

    const QByteArray field = buf.mid(off, sz);
    if (m_config.lengthPrefixEncoding == TcpLengthPrefixEncoding::DecimalAscii) {
        // 去空格后解析十进制
        QByteArray t = field.trimmed();
        if (t.isEmpty())
            return -1;
        bool convOk = false;
        const int v = t.toInt(&convOk);
        if (!convOk || v <= 0)
            return -1;
        *ok = 1;
        return v;
    }

    if (m_config.lengthPrefixEncoding == TcpLengthPrefixEncoding::BinaryBE) {
        quint32 v = 0;
        for (int i = 0; i < sz; ++i)
            v = (v << 8) | static_cast<quint8>(field.at(i));
        *ok = 1;
        return static_cast<int>(v);
    }

    // BinaryLE
    quint32 v = 0;
    for (int i = sz - 1; i >= 0; --i)
        v = (v << 8) | static_cast<quint8>(field.at(i));
    *ok = 1;
    return static_cast<int>(v);
}

QVector<QByteArray> FrameAssembler::feedLengthPrefix()
{
    QVector<QByteArray> out;
    const int maxSz = m_config.maxFrameSize > 0 ? m_config.maxFrameSize : 8192;
    const int hdrNeed = m_config.lengthPrefixOffset + m_config.lengthPrefixSize;

    while (true) {
        if (m_buffer.size() < hdrNeed)
            break;

        int ok = 0;
        const int lenField = parseLengthField(m_buffer, &ok);
        if (!ok || lenField <= 0) {
            // 无法解析：丢 1 字节尝试再同步
            m_buffer.remove(0, 1);
            continue;
        }
        // lengthIncludesHeader=true：字段即整帧长；false：字段为体长，再加头（及可选结束符）
        int totalLen = lenField;
        if (!m_config.lengthIncludesHeader) {
            totalLen = hdrNeed + lenField;
            if (m_config.hasFrameTerminator)
                ++totalLen;
        }
        if (totalLen > maxSz) {
            m_lastError = QStringLiteral("帧长度超过 maxFrameSize（%1 > %2）")
                              .arg(totalLen)
                              .arg(maxSz);
            m_buffer.clear();
            break;
        }
        if (m_buffer.size() < totalLen)
            break;

        QByteArray frame = m_buffer.left(totalLen);
        if (m_config.hasFrameTerminator) {
            if (frame.isEmpty()
                || frame.at(frame.size() - 1) != m_config.frameTerminator) {
                m_lastError = QStringLiteral("帧结束符不匹配");
                m_buffer.remove(0, 1);
                continue;
            }
        }
        m_buffer.remove(0, totalLen);
        out.push_back(frame);
    }
    return out;
}
