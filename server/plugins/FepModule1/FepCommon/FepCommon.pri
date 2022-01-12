QT -= gui
QT += network xml
QT += core

INCLUDEPATH+=$$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/FepFileOperate.h \
           $$PWD/FepProtocol.h \
           $$PWD/FepProtocolSerialize.h

SOURCES += $$PWD/FepFileOperate.cpp \
           $$PWD/FepProtocolSerialize.cpp

