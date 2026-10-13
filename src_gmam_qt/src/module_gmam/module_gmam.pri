########################################
# module_gmam.pri
# 通用报文组装模块（GMAM）— 核心：schema → 定长 QByteArray
# 版本：v0.0.3（HeaderBuilder / composeTelegram 已移出本模块）
########################################

QT += core
CONFIG += c++17

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/GmamTypes.h \
    $$PWD/GmamError.h \
    $$PWD/FieldCodec.h \
    $$PWD/DataContentBuilder.h

SOURCES += \
    $$PWD/FieldCodec.cpp \
    $$PWD/DataContentBuilder.cpp

# ---------------------------------------------------------------------------
#   GmamTypes.h          — FieldType / FieldDetail / resolve / 工厂
#   GmamError.h          — formatFieldError / 缺值占位
#   FieldCodec.h/.cpp    — parseNumericString / encodeField
#   DataContentBuilder.* — 按 schema 顺序拼接
#
# 工程侧可选（与 main.cpp 同级，非本模块）：
#   HeaderTypes.h / HeaderBuilder.* / TelegramCompose.*
# ---------------------------------------------------------------------------
