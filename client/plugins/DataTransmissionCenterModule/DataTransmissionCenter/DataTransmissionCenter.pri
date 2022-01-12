
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
    $$PWD/AddDataTransmissionCenter.ui \
    $$PWD/DataTransmissionCenter.ui

HEADERS += \
    $$PWD/AddDataTransmissionCenter.h \
    $$PWD/DTCUtility.h \
    $$PWD/DataTransmissionCenter.h \
    $$PWD/DataTransmissionModel.h \
    $$PWD/DataTransmissionCenterService.h

SOURCES += \
    $$PWD/AddDataTransmissionCenter.cpp \
    $$PWD/DTCUtility.cpp \
    $$PWD/DataTransmissionCenter.cpp \
    $$PWD/DataTransmissionModel.cpp \
    $$PWD/DataTransmissionCenterService.cpp



