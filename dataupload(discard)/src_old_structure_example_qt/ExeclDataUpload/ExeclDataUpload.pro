QT       += core sql
CONFIG += c++11 console cmdline
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# include(E:/dev/QtCreatorProject/ExcelUpload/Bin/QXlsx/Qxlsx.pri)
#qXlsx-mingw32
INCLUDEPATH += $$PWD/qXlsx-mingw64/include

LIBS += -L$$PWD/qXlsx-mingw64/lib
LIBS += -lQXlsx
SOURCES += \
    Controller.cpp \
    FileDispatcher.cpp \
    FileMoniter.cpp \
    OracleDB.cpp \
    Tools.cpp \
    XlsxReader.cpp \
    main.cpp

HEADERS += \
    Controller.h \
    FileDispatcher.h \
    FileMoniter.h \
    OracleDB.h \
    Structure.h \
    Tools.h \
    XlsxReader.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=
