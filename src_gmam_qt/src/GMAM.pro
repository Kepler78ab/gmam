QT = core

CONFIG += c++17 cmdline

include(module_gmam/module_gmam.pri)

# 已移出 module_gmam 的可选拼装工具（与 main.cpp 同级）
INCLUDEPATH += $$PWD

HEADERS += \
    HeaderTypes.h \
    HeaderBuilder.h \
    TelegramCompose.h \
    gmam_tests/GmamSelfTest.h

SOURCES += \
    HeaderBuilder.cpp \
    TelegramCompose.cpp \
    main.cpp \
    gmam_tests/GmamSelfTest.cpp

INCLUDEPATH += $$PWD/gmam_tests

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
