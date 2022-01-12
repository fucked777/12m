
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

FORMS += \
    $$PWD/AddStationResManagement.ui \
    $$PWD/StationResManagement.ui

HEADERS += \
    $$PWD/AddStationResManagement.h \
    $$PWD/SRMUtility.h \
    $$PWD/StationResManagement.h \
    $$PWD/StationResManagementModel.h \
    $$PWD/StationResManagementService.h

SOURCES += \
    $$PWD/AddStationResManagement.cpp \
    $$PWD/SRMUtility.cpp \
    $$PWD/StationResManagement.cpp \
    $$PWD/StationResManagementModel.cpp \
    $$PWD/StationResManagementService.cpp



