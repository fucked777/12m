
QT       += network xml
QT       += gui
QT       +=widgets

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
    $$PWD/AddEphemerisDataDialog.h \
    $$PWD/EDUtility.h \
    $$PWD/EphemerisData.h \
    $$PWD/EphemerisDataModel.h \
    $$PWD/EphemerisDataService.h


SOURCES += \
    $$PWD/AddEphemerisDataDialog.cpp \
    $$PWD/EDUtility.cpp \
    $$PWD/EphemerisData.cpp \
    $$PWD/EphemerisDataModel.cpp \
    $$PWD/EphemerisDataService.cpp


FORMS += \
    $$PWD/AddEphemerisDataDialog.ui \
    $$PWD/EphemerisData.ui
