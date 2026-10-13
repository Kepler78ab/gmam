QT += core gui widgets network
CONFIG += c++17
TEMPLATE = app
TARGET = TcpPeerTest

INCLUDEPATH += $$PWD

# .pro 在 tcp_peer_test/src/；构建产物按策略落在 ../build（即 tcp_peer_test/build）

FORMS += MainWindow.ui
HEADERS += MainWindow.h
SOURCES += \
    main.cpp \
    MainWindow.cpp
