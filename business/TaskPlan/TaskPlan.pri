QT       += gui widgets network printsupport xml sql

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
CONFIG += c++11

!equals(SQLGLOBALDATAPRI,$${TARGET}SQLGLOBALDATAPRI){
    SQLGLOBALDATAPRI = $${TARGET}SQLGLOBALDATAPRI
    include($$BUSINESS_INCLUDE_DIR/SqlGlobalData/SqlGlobalData.pri)
}

HEADERS += \
    $$PWD/TaskPlanSql.h

SOURCES += \
    $$PWD/TaskPlanSql.cpp


