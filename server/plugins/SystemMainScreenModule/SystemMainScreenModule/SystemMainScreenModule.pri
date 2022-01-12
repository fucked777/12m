INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Module/
INCLUDEPATH += $$PWD/Module/Model

FORMS += \
    $$PWD/Module/SystemLogWidget.ui \
    $$PWD/Module/TaskPlanWidget.ui \
    $$PWD/Module/TimeInfomationWidget.ui \
    $$PWD/SystemMainScreen.ui

HEADERS += \
    $$PWD/Module/Model/SystemLogTableModel.h \
    $$PWD/Module/SystemLogWidget.h \
    $$PWD/Module/TaskPlanWidget.h \
    $$PWD/Module/TimeInfomationWidget.h \
    $$PWD/SystemMainScreen.h \
    $$PWD/SystemMainScreenServer.h

SOURCES += \
    $$PWD/Module/Model/SystemLogTableModel.cpp \
    $$PWD/Module/SystemLogWidget.cpp \
    $$PWD/Module/TaskPlanWidget.cpp \
    $$PWD/Module/TimeInfomationWidget.cpp \
    $$PWD/SystemMainScreen.cpp \
    $$PWD/SystemMainScreenServer.cpp
