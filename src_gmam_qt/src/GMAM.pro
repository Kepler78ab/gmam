QT = core

CONFIG += c++17 cmdline

include(module_gmam/module_gmam.pri)

INCLUDEPATH += $$PWD/gmam_tests

HEADERS += \
    gmam_tests/GmamSelfTest.h

SOURCES += \
    main.cpp \
    gmam_tests/GmamSelfTest.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
