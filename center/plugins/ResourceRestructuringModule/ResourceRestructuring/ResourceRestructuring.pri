HEADERS += \
    $$PWD/Model/ResourceRestructuringTableModel.h \
    $$PWD/ResourceRestructuringService.h \
    $$PWD/ResourceRestructuringWidget.h \
    $$PWD/ResourceRestructuringDialog.h \
    $$PWD/ResourceRestructuringXMLReader.h




SOURCES += \
    $$PWD/Model/ResourceRestructuringTableModel.cpp \
    $$PWD/ResourceRestructuringService.cpp \
    $$PWD/ResourceRestructuringWidget.cpp \
    $$PWD/ResourceRestructuringDialog.cpp \
    $$PWD/ResourceRestructuringXMLReader.cpp

FORMS += \
    $$PWD/ResourceRestructuringWidget.ui \
    $$PWD/ResourceRestructuringDialog.ui


QT += core gui concurrent

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
