QT -= gui
QT += network xml
QT += core widgets

INCLUDEPATH+=$$PWD
include($$PWD/../FepCommon/FepCommon.pri)

SOURCES += \
    $$PWD/FepFileOperate.cpp \
    $$PWD/FepSendUnit.cpp \
    $$PWD/FepInitSend.cpp \
    $$PWD/FepSendData.cpp \
    $$PWD/IStateSend.cpp

HEADERS += \
    $$PWD/FepFileOperate.h \
    $$PWD/FepSendUnit.h \
    $$PWD/FepInitSend.h \
    $$PWD/FepSendData.h \
    $$PWD/IStateSend.h
