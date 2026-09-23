QT       += core sql gui widgets
CONFIG += c++11 cmdline
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# include(E:/dev/QtCreatorProject/ExcelUpload/Bin/QXlsx/Qxlsx.pri)
#qXlsx-mingw32
INCLUDEPATH += $$PWD/qXlsx-mingw64/include
INCLUDEPATH += $$PWD/libxls/include
SOURCES+=\
        $$PWD/libxls/src/xls.c\
        $$PWD/libxls/src/xlstool.c \
        $$PWD/libxls/src/endian.c \
        $$PWD/libxls/src/ole.c\
         $$PWD/libxls/src/locale.c \
    Applic.cpp \
    WidgetStatistics.cpp \
    container.cpp \
    main.cpp
win32:LIBS+=-liconv
LIBS += -L$$PWD/qXlsx-mingw64/lib
LIBS += -lQXlsx
SOURCES += \
    Controller.cpp \
    FileDispatcher.cpp \
    FileMoniter.cpp \
    OracleDB.cpp \
    Tools.cpp \
    XlsxReader.cpp \
    main2.cpp

HEADERS += \
    Applic.h \
    Controller.h \
    FileDispatcher.h \
    FileMoniter.h \
    OracleDB.h \
    Structure.h \
    Tools.h \
    WidgetStatistics.h \
    XlsxReader.h \
    container.h

FORMS += \
    WidgetStatistics.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
