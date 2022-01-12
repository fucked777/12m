QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}

SOURCES += \
    main.cpp \
    Widget.cpp \
    testInterface.cpp

HEADERS += \
    Widget.h \
    testInterface.h

FORMS += \
    Widget.ui

# 服务端和客户端公共的接口
FRAME_DIR = $$PWD/../../server
BUSINESS_INCLUDE_DIR=$$PWD/../../business
PLATFORM_INCLUDE_DIR=$$PWD/../../server/public
include($$BUSINESS_INCLUDE_DIR/Communication/Communication.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
