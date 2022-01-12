INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Model

FORMS += \
    $$PWD/SystemLogWidget.ui

HEADERS += \
    $$PWD/Model/SystemLogTableModel.h \
    $$PWD/SqlSystemLogManager.h \
    $$PWD/SystemLogService.h \
    $$PWD/SystemLogWidget.h

SOURCES += \
    $$PWD/Model/SystemLogTableModel.cpp \
    $$PWD/SqlSystemLogManager.cpp \
    $$PWD/SystemLogService.cpp \
    $$PWD/SystemLogWidget.cpp

