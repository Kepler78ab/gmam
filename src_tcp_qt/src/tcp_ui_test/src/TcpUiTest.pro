QT += core gui widgets network
CONFIG += c++17
TEMPLATE = app
TARGET = TcpUiTest

INCLUDEPATH += $$PWD

# .pro 在 tcp_ui_test/src/，module_tcp 在 ../../module_tcp/
# 构建产物按策略落在 ../build（即 tcp_ui_test/build），与 peer 互不冲突
include($$PWD/../../module_tcp/module_tcp.pri)

FORMS += MainWindow.ui
HEADERS += MainWindow.h
SOURCES += \
    main.cpp \
    MainWindow.cpp
