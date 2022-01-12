QT       += gui widgets network printsupport concurrent

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

CONFIG += c++11

SOURCES += \
    $$PWD/DynamicConfigToWidgetUtility.cpp 

HEADERS += \
    $$PWD/DynamicConfigToWidgetDefine.h \
    $$PWD/DynamicConfigToWidgetUtility.h
