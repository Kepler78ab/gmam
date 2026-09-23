#pragma once

#include <QString>

/**
 * @file GmamTypes.h
 * @brief GMAM 公共类型与字段描述（至 v0.0.3）
 *
 * FieldType 与 HeaderFieldType 本版分列。
 * overflow 默认 Error（P0）：忘记写则超长报错；FillChar 需显式配置（或用 makeNumericField）。
 */

/** 数据段字段类型 */
enum class FieldType {
    Numeric,    ///< 数值型 N：默认右对齐，不足左补 '0'；支持 scale
    Chars,      ///< 字符型 C：默认左对齐，不足右补空格
    Reserved,   ///< 预留：固定填空格，忽略 values 中的传入值
};

/** 头部字段类型（语义同 FieldType，本版独立枚举） */
enum class HeaderFieldType {
    Numeric,    ///< 同 FieldType::Numeric
    Chars,      ///< 同 FieldType::Chars
    Reserved,   ///< 同 FieldType::Reserved
};

/**
 * Numeric 小数位处理（仅 scale > 0 时有意义）
 * @note 与 OverflowPolicy::Truncate（超额截断）无关
 */
enum class Rounding {
    Truncate,   ///< 多余小数位直接丢掉
    Round,      ///< 四舍五入
};

/**
 * 编码后长度超过 length 时的超额策略（规则 7）
 * 结构体默认 Error（P0）；协议要填 9 请显式 FillChar
 */
enum class OverflowPolicy {
    Error,      ///< 记入 errors，失败
    Truncate,   ///< 按 resolve 后的 align 截断（左对齐留左，右对齐留右）
    FillChar,   ///< 整字段填 overflowFill（常用 '9'）
};

/**
 * 不足 length 时的对齐方式
 * ByType：Numeric→Right，Chars→Left
 */
enum class FieldAlign {
    ByType,     ///< 跟随字段类型的默认对齐
    Left,       ///< 左对齐，右侧补 padFill
    Right,      ///< 右对齐，左侧补 padFill
};

/**
 * 缺值判定（是否走整段 missingFill，而不调用 encode）
 */
enum class MissingPolicy {
    TreatAsMissing, ///< 无 key，或 trim 后空串 → missingFill（默认，兼容旧行为）
    EmptyIsValue,   ///< 仅无 key → missingFill；空串按正常值编码
};

/**
 * 数据段单个字段描述。
 * schema = 按拼接顺序排列的 FieldDetail 列表；各字段编码后长度恒等于 length。
 *
 * 直写示例（可只写前几项，后部用默认）：
 * @code
 * { "temp", FieldType::Numeric, 5, 3, Rounding::Truncate,
 *   OverflowPolicy::FillChar, '9',
 *   FieldAlign::ByType, '\0', '\0', MissingPolicy::TreatAsMissing }
 * @endcode
 */
struct FieldDetail {
    QString name;   ///< 字段名，与 QHash values 的 key 对应

    FieldType type = FieldType::Reserved;   ///< 字段类型

    int length = 0; ///< 固定占位字节数（必须 > 0 才参与 totalLength）

    int scale = 0;  ///< 仅 Numeric：小数放大位数，表示 10^-scale；其它类型忽略

    Rounding rounding = Rounding::Truncate; ///< 仅 Numeric：小数截断或四舍五入

    /**
     * 超额策略。默认 Error（P0）：
     * 忘记配置时超长会报错；需要填 '9' 等请设为 FillChar。
     */
    OverflowPolicy overflow = OverflowPolicy::Error;

    char overflowFill = '9'; ///< 仅 OverflowPolicy::FillChar 时使用的填充字符

    FieldAlign align = FieldAlign::ByType; ///< 不足位对齐；ByType 按 type 解析

    /**
     * 不足 length 时的补位字符。
     * '\0' 为哨兵：Numeric→'0'，Chars→空格（见 resolveFieldStyle）
     */
    char padFill = '\0';

    /**
     * 缺值（及编码失败占位）时整段填充字符。
     * '\0' 为哨兵：Numeric→'0'，Chars→空格
     */
    char missingFill = '\0';

    MissingPolicy missingPolicy = MissingPolicy::TreatAsMissing; ///< 未传 / 空串如何处理
};

/**
 * 头部单个字段描述。
 * 按 offset 写入定长缓冲区；未覆盖区间由 HeaderBuilder 的 holeFill 填充。
 * 编码规则与 FieldDetail 相同（经 toFieldDetail 转入 encodeField）。
 */
struct HeaderField {
    QString name;   ///< 字段名，与 values 的 key 对应

    HeaderFieldType type = HeaderFieldType::Reserved; ///< 字段类型

    int offset = -1; ///< 在头部缓冲区中的起始下标（从 0 起）

    int length = 0;  ///< 本字段占用字节数；须满足 offset+length ≤ 头部总长

    int scale = 0;   ///< 仅 Numeric：同 FieldDetail::scale

    Rounding rounding = Rounding::Truncate; ///< 仅 Numeric：同 FieldDetail::rounding

    OverflowPolicy overflow = OverflowPolicy::Error; ///< 同 FieldDetail；默认 Error（P0）

    char overflowFill = '9'; ///< 同 FieldDetail::overflowFill

    FieldAlign align = FieldAlign::ByType; ///< 同 FieldDetail::align

    char padFill = '\0';     ///< 同 FieldDetail::padFill（'\0' = 按类型默认）

    char missingFill = '\0'; ///< 同 FieldDetail::missingFill

    MissingPolicy missingPolicy = MissingPolicy::TreatAsMissing; ///< 同 FieldDetail
};

/**
 * resolveFieldStyle 的结果：已去掉 ByType / '\0' 哨兵后的实际样式。
 * 供 encode 补位、缺值占位、失败占位使用。
 */
struct ResolvedFieldStyle {
    FieldAlign align = FieldAlign::Left; ///< 实际对齐（不会是 ByType）
    char padFill = ' ';                  ///< 实际不足位补字符
    char missingFill = ' ';              ///< 实际缺值/失败占位字符
};

/** 将 type + align/pad/missing 哨兵解析为 ResolvedFieldStyle */
inline ResolvedFieldStyle resolveFieldStyle(FieldType type,
                                            FieldAlign align,
                                            char padFill,
                                            char missingFill)
{
    ResolvedFieldStyle r;
    if (align == FieldAlign::ByType) {
        r.align = (type == FieldType::Numeric) ? FieldAlign::Right
                                               : FieldAlign::Left;
    } else {
        r.align = align;
    }
    if (padFill == '\0')
        r.padFill = (type == FieldType::Numeric) ? '0' : ' ';
    else
        r.padFill = padFill;
    if (missingFill == '\0')
        r.missingFill = (type == FieldType::Numeric) ? '0' : ' ';
    else
        r.missingFill = missingFill;
    return r;
}

inline ResolvedFieldStyle resolveFieldStyle(const FieldDetail& f)
{
    return resolveFieldStyle(f.type, f.align, f.padFill, f.missingFill);
}

/** HeaderFieldType → FieldType（编码路径统一） */
inline FieldType toFieldType(HeaderFieldType t)
{
    switch (t) {
    case HeaderFieldType::Numeric:  return FieldType::Numeric;
    case HeaderFieldType::Chars:    return FieldType::Chars;
    case HeaderFieldType::Reserved: return FieldType::Reserved;
    }
    return FieldType::Reserved;
}

inline ResolvedFieldStyle resolveFieldStyle(const HeaderField& f)
{
    return resolveFieldStyle(toFieldType(f.type), f.align, f.padFill, f.missingFill);
}

/**
 * 是否按缺值处理（不调用 encode，直接整段 missingFill）。
 * @param keyPresent values 中是否存在该字段名
 * @param value      key 存在时的字符串（可为空）
 */
inline bool shouldUseMissingFill(MissingPolicy policy,
                                 bool keyPresent,
                                 const QString& value)
{
    if (!keyPresent)
        return true;
    if (policy == MissingPolicy::TreatAsMissing && value.trimmed().isEmpty())
        return true;
    return false;
}

/**
 * @name 可选工厂
 * Numeric 默认 FillChar('9')；Chars 默认 Error。
 * align/pad/missing 使用结构体默认（ByType / 哨兵 / TreatAsMissing）。
 * @{
 */
inline FieldDetail makeNumericField(const QString& name, int length,
                                    int scale = 0,
                                    Rounding rounding = Rounding::Truncate,
                                    OverflowPolicy overflow = OverflowPolicy::FillChar,
                                    char overflowFill = '9')
{
    FieldDetail f;
    f.name = name;
    f.type = FieldType::Numeric;
    f.length = length;
    f.scale = scale;
    f.rounding = rounding;
    f.overflow = overflow;
    f.overflowFill = overflowFill;
    return f;
}

inline FieldDetail makeCharsField(const QString& name, int length,
                                  OverflowPolicy overflow = OverflowPolicy::Error,
                                  char overflowFill = '9')
{
    FieldDetail f;
    f.name = name;
    f.type = FieldType::Chars;
    f.length = length;
    f.overflow = overflow;
    f.overflowFill = overflowFill;
    return f;
}

inline FieldDetail makeReservedField(const QString& name, int length)
{
    FieldDetail f;
    f.name = name;
    f.type = FieldType::Reserved;
    f.length = length;
    return f;
}

inline HeaderField makeHeaderNumeric(const QString& name, int offset, int length,
                                     int scale = 0,
                                     Rounding rounding = Rounding::Truncate,
                                     OverflowPolicy overflow = OverflowPolicy::FillChar,
                                     char overflowFill = '9')
{
    HeaderField f;
    f.name = name;
    f.type = HeaderFieldType::Numeric;
    f.offset = offset;
    f.length = length;
    f.scale = scale;
    f.rounding = rounding;
    f.overflow = overflow;
    f.overflowFill = overflowFill;
    return f;
}

inline HeaderField makeHeaderChars(const QString& name, int offset, int length,
                                   OverflowPolicy overflow = OverflowPolicy::Error,
                                   char overflowFill = '9')
{
    HeaderField f;
    f.name = name;
    f.type = HeaderFieldType::Chars;
    f.offset = offset;
    f.length = length;
    f.overflow = overflow;
    f.overflowFill = overflowFill;
    return f;
}

inline HeaderField makeHeaderReserved(const QString& name, int offset, int length)
{
    HeaderField f;
    f.name = name;
    f.type = HeaderFieldType::Reserved;
    f.offset = offset;
    f.length = length;
    return f;
}
/** @} */
