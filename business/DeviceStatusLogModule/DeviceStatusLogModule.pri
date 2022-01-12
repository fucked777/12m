INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Model

FORMS += \
    $$PWD/DeviceStatusLogWidget.ui

HEADERS += \
    $$PWD/DeviceStatusLogService.h \
    $$PWD/DeviceStatusLogWidget.h \
    $$PWD/Model/DeviceStatusLogTableModel.h \
    $$PWD/SqlDeviceStatusManager.h

SOURCES += \
    $$PWD/DeviceStatusLogService.cpp \
    $$PWD/DeviceStatusLogWidget.cpp \
    $$PWD/Model/DeviceStatusLogTableModel.cpp \
    $$PWD/SqlDeviceStatusManager.cpp
