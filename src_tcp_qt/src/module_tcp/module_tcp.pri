########################################
# module_tcp.pri
# TCP 通讯模块 — P0+P1
########################################

QT += core network
CONFIG += c++17

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/profiles

HEADERS += \
    $$PWD/TcpTypes.h \
    $$PWD/TcpClientConfig.h \
    $$PWD/IFrameMatcher.h \
    $$PWD/SendQueue.h \
    $$PWD/FrameAssembler.h \
    $$PWD/TcpClientWorker.h \
    $$PWD/TcpClient.h \
    $$PWD/profiles/PesFrameHints.h

SOURCES += \
    $$PWD/SendQueue.cpp \
    $$PWD/FrameAssembler.cpp \
    $$PWD/TcpClientWorker.cpp \
    $$PWD/TcpClient.cpp
