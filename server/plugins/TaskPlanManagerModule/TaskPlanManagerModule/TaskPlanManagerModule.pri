INCLUDEPATH += $$PWD

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}
HEADERS += \
    $$PWD/ParseDeviceWorkTaskThread.h \
    $$PWD/TaskPlanManagerService.h \
    $$PWD/TaskPlanManagerXmlReader.h

SOURCES += \
    $$PWD/ParseDeviceWorkTaskThread.cpp \
    $$PWD/TaskPlanManagerService.cpp \
    $$PWD/TaskPlanManagerXmlReader.cpp
