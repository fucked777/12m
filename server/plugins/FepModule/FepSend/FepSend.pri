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
    $$PWD/FepSendUnit.cpp

HEADERS += \
    $$PWD/FepSendUnit.h
