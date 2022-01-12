QT -= gui
QT += network xml
QT += core

INCLUDEPATH+=$$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/FepProtocol.h \
           $$PWD/FepProtocolSerialize.h

SOURCES += $$PWD/FepProtocolSerialize.cpp

