QT       += core gui concurrent xml

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/QCustomPlot

CONFIG += c++11


HEADERS += \
    $$PWD/QCustomPlot/qcustomplot.h

SOURCES += \
     $$PWD/QCustomPlot/qcustomplot.cpp
