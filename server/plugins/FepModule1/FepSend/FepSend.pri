QT -= gui
QT += network xml
QT += core widgets

INCLUDEPATH+=$$PWD
DEPENDPATH += $$PWD

!equals(FEPCOMMONPRI,$${TARGET}FEPCOMMONPRI){
    FEPCOMMONPRI = $${TARGET}FEPCOMMONPRI
    include($$PWD/../FepCommon/FepCommon.pri)
}


SOURCES += \
    $$PWD/FepSendUnit.cpp \
    $$PWD/FepInitSend.cpp \
    $$PWD/FepSendData.cpp \
    $$PWD/IStateSend.cpp

HEADERS += \
    $$PWD/FepSendUnit.h \
    $$PWD/FepInitSend.h \
    $$PWD/FepSendData.h \
    $$PWD/IStateSend.h
