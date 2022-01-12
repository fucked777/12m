
INCLUDEPATH += $$PWD \
            $$PWD/entity \
            $$PWD/Model

HEADERS += \
    $$PWD/Model/UserTableModel.h \
    $$PWD/UserLoginWidget.h \
    $$PWD/UserManagerService.h \
    $$PWD/UserModifyWidget.h \
    $$PWD/UserRegisterWidget.h \
    $$PWD/UserListWidget.h

SOURCES += \
    $$PWD/Model/UserTableModel.cpp \
    $$PWD/UserLoginWidget.cpp \
    $$PWD/UserManagerService.cpp \
    $$PWD/UserModifyWidget.cpp \
    $$PWD/UserRegisterWidget.cpp \
    $$PWD/UserListWidget.cpp

FORMS += \
    $$PWD/UserLoginWidget.ui \
    $$PWD/UserModifyWidget.ui \
    $$PWD/UserRegisterWidget.ui \
    $$PWD/UserListWidget.ui

QT += concurrent
