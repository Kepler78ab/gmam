#include "GmamSelfTest.h"

#include "DataContentBuilder.h"
#include "HeaderBuilder.h"
#include "TelegramCompose.h"

#include <QDebug>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <cstring>

// ---------------------------------------------------------------------------
// 旧 ExeclDataUpload 风格结构体（与 Structure.h 对齐，仅用于比对）
// ---------------------------------------------------------------------------
namespace {

#pragma pack(push, 1)
struct LegacyTestData {
    char m_element[10];
    char m_resultOfElement[10];
    char m_unit[20];
    LegacyTestData() { memset(this, ' ', sizeof(LegacyTestData)); }
};

struct LegacyT9PhyToErp {
    char m_formId[10];
    char m_inputCode;
    char m_lotNo[20];
    char m_HeatNo[10];
    char m_testNum;
    char m_testSeq;
    char m_shiftWork[6];
    char m_sampStatus[2];
    char m_sampleId[20];
    char m_testType;
    char m_couponCode[10];
    char m_numberOfAnalysisElements[2];
    LegacyTestData m_data[9];
    char m_end;

    LegacyT9PhyToErp()
    {
        memset(this, ' ', sizeof(LegacyT9PhyToErp));
        m_end = 0;
    }
};
#pragma pack(pop)

static_assert(sizeof(LegacyTestData) == 40, "LegacyTestData size");
static_assert(sizeof(LegacyT9PhyToErp) == 445, "LegacyT9PhyToErp size");

struct T9AssembleInput {
    QString formId;
    QString sampleId;
    QString unit;
    QStringList rawValues;
    double minVal = 0.0;
    double maxVal = 1e9;
};

void legacyCopyToFixedBuffer(char* dest, const QString& src, int len)
{
    // 旧逻辑：toUtf8 + 左贴截断，其余保持原 memset 空格
    const QByteArray ba = src.toUtf8();
    const int actualLen = qMin(ba.length(), len);
    memcpy(dest, ba.constData(), actualLen);
}

void legacyFillTestData(LegacyTestData& target,
                        const QString& name,
                        const QString& value,
                        const QString& unit)
{
    legacyCopyToFixedBuffer(target.m_element, name, 10);
    legacyCopyToFixedBuffer(target.m_resultOfElement, value, 10);
    legacyCopyToFixedBuffer(target.m_unit, unit, 20);
}

// 模仿 ExcelOperator::fillT9Data → 结构体内存镜像
QByteArray assembleT9Legacy(const T9AssembleInput& in, QString& error)
{
    error.clear();
    LegacyT9PhyToErp t9;

    legacyCopyToFixedBuffer(t9.m_formId, in.formId, 10);
    t9.m_inputCode = 'N';
    legacyCopyToFixedBuffer(t9.m_sampleId, in.sampleId, 20);

    QStringList validValues;
    for (const QString& str : in.rawValues) {
        bool ok = false;
        const double val = str.toDouble(&ok);
        if (ok && val >= in.minVal && val <= in.maxVal)
            validValues << str;
    }

    const int totalValid = validValues.size();
    if (totalValid < 3) {
        error = QStringLiteral("符合条件的数据不足 3 个");
        return {};
    }

    const int count = qMin(totalValid, 9);
    const QString countStr = QString::number(count).rightJustified(2, QLatin1Char(' '));
    legacyCopyToFixedBuffer(t9.m_numberOfAnalysisElements, countStr, 2);

    for (int i = 0; i < count; ++i) {
        const double dVal = validValues.at(i).toDouble();
        const QString formattedVal =
            QString::number(dVal, 'f', 3).leftJustified(10, QLatin1Char(' '));
        const QString elementName = QStringLiteral("D01%1").arg(i + 1);
        legacyFillTestData(t9.m_data[i], elementName, formattedVal, in.unit);
    }

    return QByteArray(reinterpret_cast<const char*>(&t9),
                      static_cast<int>(sizeof(LegacyT9PhyToErp)));
}

QVector<FieldDetail> makeT9GmamSchema()
{
    // name, type, length, scale, rounding, overflow, overflowFill
    // 旧风格字段一律 Chars + Truncate；未采集位用 Reserved
    return QVector<FieldDetail>{
        { QStringLiteral("formId"),                     FieldType::Chars,    10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("inputCode"),                  FieldType::Chars,     1, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("lotNo"),                      FieldType::Reserved, 20, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("HeatNo"),                     FieldType::Reserved, 10, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("testNum"),                    FieldType::Reserved,  1, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("testSeq"),                    FieldType::Reserved,  1, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("shiftWork"),                  FieldType::Reserved,  6, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("sampStatus"),                 FieldType::Reserved,  2, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("sampleId"),                   FieldType::Chars,    20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("testType"),                   FieldType::Reserved,  1, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("couponCode"),                 FieldType::Reserved, 10, 0, Rounding::Truncate, OverflowPolicy::Error,    '9' },
        { QStringLiteral("numberOfAnalysisElements"),   FieldType::Chars,     2, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },

        { QStringLiteral("element_0"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_0"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_0"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_1"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_1"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_1"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_2"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_2"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_2"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_3"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_3"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_3"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_4"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_4"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_4"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_5"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_5"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_5"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_6"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_6"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_6"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_7"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_7"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_7"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("element_8"), FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("result_8"),  FieldType::Chars, 10, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
        { QStringLiteral("unit_8"),    FieldType::Chars, 20, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },

        { QStringLiteral("end"), FieldType::Chars, 1, 0, Rounding::Truncate, OverflowPolicy::Truncate, '9' },
    };
}

// 同一业务输入 → GMAM DataContentBuilder（Chars + Truncate）
QByteArray assembleT9Gmam(const T9AssembleInput& in, QString& error)
{
    error.clear();

    QStringList validValues;
    for (const QString& str : in.rawValues) {
        bool ok = false;
        const double val = str.toDouble(&ok);
        if (ok && val >= in.minVal && val <= in.maxVal)
            validValues << str;
    }
    const int totalValid = validValues.size();
    if (totalValid < 3) {
        error = QStringLiteral("符合条件的数据不足 3 个");
        return {};
    }
    const int count = qMin(totalValid, 9);

    QHash<QString, QString> values;
    values.insert(QStringLiteral("formId"), in.formId);
    values.insert(QStringLiteral("inputCode"), QStringLiteral("N"));
    values.insert(QStringLiteral("sampleId"), in.sampleId);
    values.insert(QStringLiteral("numberOfAnalysisElements"),
                  QString::number(count).rightJustified(2, QLatin1Char(' ')));

    for (int i = 0; i < count; ++i) {
        const double dVal = validValues.at(i).toDouble();
        const QString formattedVal =
            QString::number(dVal, 'f', 3).leftJustified(10, QLatin1Char(' '));
        values.insert(QStringLiteral("element_%1").arg(i),
                      QStringLiteral("D01%1").arg(i + 1));
        values.insert(QStringLiteral("result_%1").arg(i), formattedVal);
        values.insert(QStringLiteral("unit_%1").arg(i), in.unit);
    }
    values.insert(QStringLiteral("end"), QString(QChar(0)));

    DataContentBuilder builder(makeT9GmamSchema());
    QStringList errors;
    const QByteArray out = builder.build(values, errors);
    if (!errors.isEmpty()) {
        error = errors.join(QLatin1Char(';'));
        return {};
    }
    return out;
}

T9AssembleInput sampleT9Input()
{
    T9AssembleInput in;
    in.formId = QStringLiteral("T9PHYTOERP");
    in.sampleId = QStringLiteral("C42400729110A");
    in.unit = QStringLiteral("MPa");
    in.minVal = 0.0;
    in.maxVal = 1000.0;
    in.rawValues = QStringList{
        QStringLiteral("12.34"),
        QStringLiteral("13.5"),
        QStringLiteral("11.2"),
        QStringLiteral("9999"), // 超 max，旧逻辑剔除
    };
    return in;
}

void logFirstDiff(const QByteArray& a, const QByteArray& b)
{
    const int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        if (a.at(i) != b.at(i)) {
            qWarning() << "first diff at" << i
                       << "legacy=" << static_cast<int>(static_cast<uchar>(a.at(i)))
                       << "gmam=" << static_cast<int>(static_cast<uchar>(b.at(i)));
            return;
        }
    }
    if (a.size() != b.size())
        qWarning() << "size differs: legacy=" << a.size() << "gmam=" << b.size();
}

// 空格→·，NUL→\0，便于肉眼看定长报文
QString bytesForEye(const QByteArray& ba)
{
    QString s;
    s.reserve(ba.size() * 2);
    for (unsigned char c : ba) {
        if (c == 0)
            s += QStringLiteral("\\0");
        else if (c == ' ')
            s += QChar(0x00B7); // ·
        else if (c >= 32 && c < 127)
            s += QLatin1Char(static_cast<char>(c));
        else
            s += QStringLiteral("\\x%1").arg(c, 2, 16, QLatin1Char('0'));
    }
    return s;
}

void printT9Payload(const char* tag, const QByteArray& ba)
{
    qInfo().noquote() << "----" << tag << "len=" << ba.size() << "----";
    if (ba.size() < 445) {
        qInfo().noquote() << bytesForEye(ba);
        return;
    }
    qInfo().noquote() << "formId[0..10) :" << bytesForEye(ba.mid(0, 10));
    qInfo().noquote() << "inputCode     :" << bytesForEye(ba.mid(10, 1));
    qInfo().noquote() << "lotNo         :" << bytesForEye(ba.mid(11, 20));
    qInfo().noquote() << "HeatNo        :" << bytesForEye(ba.mid(31, 10));
    qInfo().noquote() << "testNum/Seq   :" << bytesForEye(ba.mid(41, 2));
    qInfo().noquote() << "shiftWork     :" << bytesForEye(ba.mid(43, 6));
    qInfo().noquote() << "sampStatus    :" << bytesForEye(ba.mid(49, 2));
    qInfo().noquote() << "sampleId      :" << bytesForEye(ba.mid(51, 20));
    qInfo().noquote() << "testType      :" << bytesForEye(ba.mid(71, 1));
    qInfo().noquote() << "couponCode    :" << bytesForEye(ba.mid(72, 10));
    qInfo().noquote() << "elemCount     :" << bytesForEye(ba.mid(82, 2));
    for (int i = 0; i < 9; ++i) {
        const int off = 84 + i * 40;
        qInfo().noquote() << QStringLiteral("data[%1]        :").arg(i)
                          << bytesForEye(ba.mid(off, 40));
    }
    qInfo().noquote() << "end           :" << bytesForEye(ba.mid(444, 1));
    qInfo().noquote() << "full          :" << bytesForEye(ba);
}

} // namespace

// ---------------------------------------------------------------------------
// 冒烟 + 多错误收集（v0.0.2）
// ---------------------------------------------------------------------------
bool gmamSmokeTest()
{
    QStringList errors;

    QVector<FieldDetail> dataSchema = {
        makeNumericField(QStringLiteral("temp"), 5, 3, Rounding::Truncate),
        makeNumericField(QStringLiteral("volt"), 4, 1, Rounding::Round),
        makeCharsField(QStringLiteral("id"), 6),
        makeReservedField(QStringLiteral("reserve"), 3),
        makeCharsField(QStringLiteral("note"), 5, OverflowPolicy::Truncate),
        makeNumericField(QStringLiteral("big"), 3, 0, Rounding::Truncate,
                         OverflowPolicy::FillChar),
    };
    DataContentBuilder dataBuilder(dataSchema);

    QHash<QString, QString> values;
    values.insert(QStringLiteral("temp"), QStringLiteral("53.167"));
    values.insert(QStringLiteral("volt"), QStringLiteral("12.35"));
    values.insert(QStringLiteral("id"), QStringLiteral("z01"));
    values.insert(QStringLiteral("note"), QStringLiteral("abcdef"));
    values.insert(QStringLiteral("big"), QStringLiteral("12345"));

    const QByteArray data = dataBuilder.build(values, errors);
    if (data.isEmpty() || !errors.isEmpty()) {
        qWarning() << "data build failed:" << errors;
        return false;
    }
    const QByteArray expectData =
        QByteArray("53167") + "0124" + "z01   " + "   " + "abcde" + "999";
    if (data != expectData) {
        qWarning() << "data mismatch:" << data << "expected:" << expectData;
        return false;
    }
    qInfo() << "data OK, len=" << data.size() << data;

    QVector<HeaderField> headerSchema = {
        makeHeaderChars(QStringLiteral("sync"), 0, 2),
        makeHeaderNumeric(QStringLiteral("cmd"), 2, 1),
        makeHeaderReserved(QStringLiteral("pad"), 7, 3),
    };
    HeaderBuilder headerBuilder(headerSchema, 10, ' ');
    QHash<QString, QString> hvals;
    hvals.insert(QStringLiteral("sync"), QStringLiteral("AB"));
    hvals.insert(QStringLiteral("cmd"), QStringLiteral("7"));
    const QByteArray head = headerBuilder.build(hvals, errors);
    if (head.isEmpty() || !errors.isEmpty()) {
        qWarning() << "header build failed:" << errors;
        return false;
    }
    if (head != QByteArray("AB7       ")) {
        qWarning() << "header mismatch:" << head.toHex() << head;
        return false;
    }
    qInfo() << "header OK, len=" << head.size() << head;

    const QByteArray telegram = composeTelegram(head, data);
    if (telegram.size() != head.size() + data.size() + 1
        || telegram.back() != static_cast<char>(0x0A)) {
        qWarning() << "compose failed, size=" << telegram.size();
        return false;
    }
    qInfo() << "compose OK, total=" << telegram.size();

    // 单字段超长 Error
    DataContentBuilder strictBuilder({
        makeCharsField(QStringLiteral("id"), 3, OverflowPolicy::Error),
    });
    QHash<QString, QString> bad;
    bad.insert(QStringLiteral("id"), QStringLiteral("toolong"));
    const QByteArray badOut = strictBuilder.build(bad, errors);
    if (!badOut.isEmpty() || errors.size() != 1) {
        qWarning() << "expected 1 chars overflow error, got" << errors;
        return false;
    }
    qInfo() << "chars Error OK:" << errors;

    // 多字段同时失败 → 收集 ≥2 条
    DataContentBuilder multiBuilder({
        makeNumericField(QStringLiteral("n1"), 3, 0, Rounding::Truncate,
                         OverflowPolicy::Error),
        makeCharsField(QStringLiteral("c1"), 2, OverflowPolicy::Error),
        makeNumericField(QStringLiteral("n2"), 2, 0, Rounding::Truncate,
                         OverflowPolicy::FillChar),
    });
    QHash<QString, QString> multiVals;
    multiVals.insert(QStringLiteral("n1"), QStringLiteral("abcd"));
    multiVals.insert(QStringLiteral("c1"), QStringLiteral("xyz"));
    multiVals.insert(QStringLiteral("n2"), QStringLiteral("99"));
    const QByteArray multiOut = multiBuilder.build(multiVals, errors);
    if (!multiOut.isEmpty() || errors.size() < 2) {
        qWarning() << "expected >=2 errors, got" << errors
                   << "outLen=" << multiOut.size();
        return false;
    }
    qInfo() << "multi errors OK, count=" << errors.size();
    for (const QString& e : errors)
        qInfo().noquote() << "  -" << e;

    // --- v0.0.3：右对齐补空格 / 自定义 missingFill / EmptyIsValue ---
    {
        FieldDetail countFld;
        countFld.name = QStringLiteral("cnt");
        countFld.type = FieldType::Chars;
        countFld.length = 2;
        countFld.overflow = OverflowPolicy::Truncate;
        countFld.align = FieldAlign::Right;
        countFld.padFill = ' ';
        DataContentBuilder b({countFld});
        QHash<QString, QString> v;
        v.insert(QStringLiteral("cnt"), QStringLiteral("3"));
        const QByteArray out = b.build(v, errors);
        if (out != QByteArray(" 3") || !errors.isEmpty()) {
            qWarning() << "align Right failed:" << out << errors;
            return false;
        }
        qInfo() << "align Right OK:" << out;
    }
    {
        FieldDetail code;
        code.name = QStringLiteral("code");
        code.type = FieldType::Chars;
        code.length = 4;
        code.missingFill = '*';
        DataContentBuilder b({code});
        QHash<QString, QString> v; // 不传 code
        const QByteArray out = b.build(v, errors);
        if (out != QByteArray("****") || !errors.isEmpty()) {
            qWarning() << "missingFill failed:" << out << errors;
            return false;
        }
        qInfo() << "missingFill OK:" << out;
    }
    {
        FieldDetail nameFld;
        nameFld.name = QStringLiteral("name");
        nameFld.type = FieldType::Chars;
        nameFld.length = 4;
        nameFld.missingPolicy = MissingPolicy::EmptyIsValue;
        nameFld.missingFill = '*'; // 未传才用 *
        DataContentBuilder b({nameFld});
        QHash<QString, QString> v;
        v.insert(QStringLiteral("name"), QString()); // 空串当值 → 全空格 pad
        const QByteArray out = b.build(v, errors);
        if (out != QByteArray("    ") || !errors.isEmpty()) {
            qWarning() << "EmptyIsValue failed:" << out << errors;
            return false;
        }
        qInfo() << "EmptyIsValue OK:" << out;
    }

    return true;
}

// ---------------------------------------------------------------------------
// 旧拼装 vs GMAM：同一输入，逐字节比对
// ---------------------------------------------------------------------------
bool gmamCompareT9LegacyVsGmam()
{
    const T9AssembleInput in = sampleT9Input();
    QString errLegacy;
    QString errGmam;

    const QByteArray legacy = assembleT9Legacy(in, errLegacy);
    if (legacy.isEmpty()) {
        qWarning() << "legacy assemble failed:" << errLegacy;
        return false;
    }

    const QByteArray gmam = assembleT9Gmam(in, errGmam);
    if (gmam.isEmpty()) {
        qWarning() << "gmam assemble failed:" << errGmam;
        return false;
    }

    qInfo() << "T9 legacy len=" << legacy.size() << "gmam len=" << gmam.size();
    printT9Payload("LEGACY (旧拼装)", legacy);
    printT9Payload("GMAM   (新拼装)", gmam);

    if (legacy != gmam) {
        qWarning() << "T9 legacy vs gmam MISMATCH";
        logFirstDiff(legacy, gmam);
        return false;
    }

    qInfo() << "T9 legacy vs gmam MATCH, len=" << legacy.size();
    return true;
}

bool gmamRunAllTests()
{
    bool ok = true;
    if (!gmamSmokeTest()) {
        qWarning() << "gmamSmokeTest FAILED";
        ok = false;
    } else {
        qInfo() << "gmamSmokeTest PASSED";
    }

    if (!gmamCompareT9LegacyVsGmam()) {
        qWarning() << "gmamCompareT9LegacyVsGmam FAILED";
        ok = false;
    } else {
        qInfo() << "gmamCompareT9LegacyVsGmam PASSED";
    }
    return ok;
}
