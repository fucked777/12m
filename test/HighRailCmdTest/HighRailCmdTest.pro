QT       += network xml
QT       += gui
QT       += widgets
QT       += core gui


include($$PWD/PDXP/PDXP.pri)
TARGET = HighRailCmdTest

SOURCES += NetInterfaceHelper.cpp \
    CConverter.cpp \
    Endian.cpp \
    StationNetCenterCmdTest.cpp \
    Utility.cpp \
    main.cpp


HEADERS += NetInterfaceHelper.h \
        CConverter.h \
        Endian.h \
        ErrorCode.h \
        StationNetCenterCmdTest.h \
        Utility.h
FORMS += \
    StationNetCenterCmdTest.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}

