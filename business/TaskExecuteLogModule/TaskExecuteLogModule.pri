INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Model

FORMS += \
    $$PWD/TaskExecuteLogWidget.ui

HEADERS += \
    $$PWD/Model/AutoTaskLogTableModel.h \
    $$PWD/Model/TaskPlanTableModel.h \
    $$PWD/SqlTaskExecuteManager.h \
    $$PWD/TaskExecuteLogService.h \
    $$PWD/TaskExecuteLogWidget.h

SOURCES += \
    $$PWD/Model/AutoTaskLogTableModel.cpp \
    $$PWD/Model/TaskPlanTableModel.cpp \
    $$PWD/SqlTaskExecuteManager.cpp \
    $$PWD/TaskExecuteLogService.cpp \
    $$PWD/TaskExecuteLogWidget.cpp
