#ifndef GMAM_HEADERTYPES_H
#define GMAM_HEADERTYPES_H

#include "GmamTypes.h"

/**
 * @file HeaderTypes.h
 * @brief 头部 schema 类型（已移出 module_gmam，供演示/兼容工程使用）
 *
 * 连续无空洞的头也可直接用另一套 FieldDetail + DataContentBuilder。
 */

/** 头部字段类型（语义同 FieldType） */
enum class HeaderFieldType {
    Numeric,
    Chars,
    Reserved,
};

/**
 * 头部单个字段描述：按 offset 写入定长缓冲区。
 * 编码规则与 FieldDetail 相同。
 */
struct HeaderField {
    QString name;
    HeaderFieldType type = HeaderFieldType::Reserved;
    int offset = -1;
    int length = 0;
    int scale = 0;
    Rounding rounding = Rounding::Truncate;
    OverflowPolicy overflow = OverflowPolicy::Error;
    char overflowFill = '9';
    FieldAlign align = FieldAlign::ByType;
    char padFill = '\0';
    char missingFill = '\0';
    MissingPolicy missingPolicy = MissingPolicy::TreatAsMissing;
};

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

inline FieldDetail toFieldDetail(const HeaderField& f)
{
    FieldDetail d;
    d.name          = f.name;
    d.type          = toFieldType(f.type);
    d.length        = f.length;
    d.scale         = f.scale;
    d.rounding      = f.rounding;
    d.overflow      = f.overflow;
    d.overflowFill  = f.overflowFill;
    d.align         = f.align;
    d.padFill       = f.padFill;
    d.missingFill   = f.missingFill;
    d.missingPolicy = f.missingPolicy;
    return d;
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

#endif // GMAM_HEADERTYPES_H
