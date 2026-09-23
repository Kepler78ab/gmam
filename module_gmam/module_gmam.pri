########################################
# module_gmam.pri
# 通用报文组装模块（GMAM）
# 版本：v0.0.1（已审定计划，见 docs/v0.0.1）
########################################

QT += core
CONFIG += c++17

INCLUDEPATH += $$PWD

# ---------------------------------------------------------------------------
# 源文件清单（与 docs/v0.0.1 计划包一致）
# 实现前文件可能尚不存在；接入 GMAM.pro 前请先落地对应 .h/.cpp
# ---------------------------------------------------------------------------

HEADERS += \
    $$PWD/GmamTypes.h \
    $$PWD/GmamError.h \
    $$PWD/FieldCodec.h \
    $$PWD/DataContentBuilder.h \
    $$PWD/HeaderBuilder.h \
    $$PWD/TelegramCompose.h

SOURCES += \
    $$PWD/FieldCodec.cpp \
    $$PWD/DataContentBuilder.cpp \
    $$PWD/HeaderBuilder.cpp \
    $$PWD/TelegramCompose.cpp

# ---------------------------------------------------------------------------
# 文件职责速查
#   GmamTypes.h          — FieldType / HeaderFieldType（分列）/
#                          Rounding / OverflowPolicy /
#                          FieldDetail / HeaderField
#   GmamError.h          — formatFieldError / failurePlaceholder（v0.0.2）
#   FieldCodec.h/.cpp    — parseNumericString / encodeField
#   DataContentBuilder.* — 顺序拼接；QStringList 错误收集
#   HeaderBuilder.*      — 按 offset 填头部；QStringList 错误收集
#   TelegramCompose.*    — composeTelegram(head, data, 0x0A)
#
# v0.0.3：对齐跟随 resolve(align)；失败/缺值占位用 missingFill
# overflow：P0（默认 Error，不自动改 FillChar）
# ---------------------------------------------------------------------------
