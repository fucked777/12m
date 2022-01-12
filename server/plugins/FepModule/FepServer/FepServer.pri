QT -= gui
QT += network xml
QT += core widgets

INCLUDEPATH+=$$PWD

!equals(FEPCOMMONPRI,$${TARGET}FEPCOMMONPRI){
    FEPCOMMONPRI = $${TARGET}FEPCOMMONPRI
    include($$PWD/../FepCommon/FepCommon.pri)
}


SOURCES += \
    $$PWD/FepServer.cpp \
    $$PWD/FepTcpServer.cpp \
    $$PWD/FepTcpSocket.cpp



HEADERS += \
    $$PWD/FepServer.h \
    $$PWD/FepTcpServer.h \
    $$PWD/FepTcpSocket.h

