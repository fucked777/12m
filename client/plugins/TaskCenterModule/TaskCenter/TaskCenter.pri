
QT       += network xml
QT       += gui
QT       +=widgets

# 没有定义此变量,证明JSON没有引用
!equals(JSONPRI,$${TARGET}JSONPRI){
    JSONPRI = $${TARGET}JSONPRI
    include($$BUSINESS_INCLUDE_DIR/RapidJson/RapidJson.pri)
}
!equals(UTILPRI,$${TARGET}UTILPRI){
    UTILPRI = $${TARGET}UTILPRI
    include($$BUSINESS_INCLUDE_DIR/Util/Util.pri)
}

!equals(REDISCLIPRI,$${TARGET}REDISCLIPRI){
    REDISCLIPRI = $${TARGET}REDISCLIPRI
    include($$BUSINESS_INCLUDE_DIR/RedisCli/RedisCli.pri)
}

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/AddTaskCenterDialog.h \
    $$PWD/TCUtility.h \
    $$PWD/TaskCenter.h \
    $$PWD/TaskCenterModel.h \
    $$PWD/TaskCenterService.h

SOURCES += \
    $$PWD/AddTaskCenterDialog.cpp \
    $$PWD/TCUtility.cpp \
    $$PWD/TaskCenter.cpp \
    $$PWD/TaskCenterModel.cpp \
    $$PWD/TaskCenterService.cpp

FORMS += \
    $$PWD/AddTaskCenterDialog.ui \
    $$PWD/TaskCenter.ui


