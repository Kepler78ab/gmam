#ifndef MODULE_GMAM_GMAMTYPES_H
#define MODULE_GMAM_GMAMTYPES_H

#include <QString>

/**
 * @file GmamTypes.h
 * @brief GMAM 核心类型与字段描述（至 v0.0.3）
 *
 * 模块职责：按 FieldDetail schema 把「字段名→字符串」编成定长 QByteArray。
 * 头部 / 整包拼接不在本模块（见工程内 HeaderBuilder / TelegramCompose）。
 * overflow 默认 Error（P0）。
 */

/** 字段类型 */
enum class FieldType {
    Numeric,    ///< 数值型 N：默认右对齐，不足左补 '0'；支持 scale
    Chars,      ///< 字符型 C：默认左对齐，不足右补空格
    Reserved,   ///< 预留：固定填空格，忽略 values 中的传入值
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
    TreatAsMissing, ///< 无 key，或 trim 后空串 → missingFill（默认）
    EmptyIsValue,   ///< 仅无 key → missingFill；空串按正常值编码
};

/**
 * 单个字段描述。
 * schema = 按拼接顺序排列的 FieldDetail 列表；各字段编码后长度恒等于 length。
 */
struct FieldDetail {
    QString name;   ///< 字段名，与 QHash values 的 key 对应

    FieldType type = FieldType::Reserved;

    int length = 0; ///< 固定占位字节数（必须 > 0 才参与 totalLength）

    int scale = 0;  ///< 仅 Numeric：小数放大位数

    Rounding rounding = Rounding::Truncate;

    OverflowPolicy overflow = OverflowPolicy::Error;

    char overflowFill = '9';

    FieldAlign align = FieldAlign::ByType;

    /** '\0' 为哨兵：Numeric→'0'，Chars→空格 */
    char padFill = '\0';

    /** '\0' 为哨兵：Numeric→'0'，Chars→空格 */
    char missingFill = '\0';

    MissingPolicy missingPolicy = MissingPolicy::TreatAsMissing;
};

struct ResolvedFieldStyle {
    FieldAlign align = FieldAlign::Left;
    char padFill = ' ';
    char missingFill = ' ';
};

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

#endif // MODULE_GMAM_GMAMTYPES_H
