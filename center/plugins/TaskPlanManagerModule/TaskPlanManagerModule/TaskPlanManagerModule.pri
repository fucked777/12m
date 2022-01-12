INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/Model
DEPENDPATH += $$PWD/Model



RESOURCES += \
    $$PWD/res.qrc

FORMS += \
    $$PWD/DeviceWorkTaskWidget.ui \
    $$PWD/LocalPlanDialog.ui \
    $$PWD/PlanListWidget.ui

HEADERS += \
    $$PWD/DataTransferTaskModel.h \
    $$PWD/DeviceWorkTaskModel.h \
    $$PWD/DeviceWorkTaskWidget.h \
    $$PWD/LocalPlanDialog.h \
    $$PWD/PlanListWidget.h \
    $$PWD/RealTimeLogModel.h \
    $$PWD/TaskPlanManagerService.h

SOURCES += \
    $$PWD/DataTransferTaskModel.cpp \
    $$PWD/DeviceWorkTaskModel.cpp \
    $$PWD/DeviceWorkTaskWidget.cpp \
    $$PWD/LocalPlanDialog.cpp \
    $$PWD/PlanListWidget.cpp \
    $$PWD/RealTimeLogModel.cpp \
    $$PWD/TaskPlanManagerService.cpp
