
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
    $$PWD/DMEditDialog.h \
    $$PWD/DMItemDataModel.h \
    $$PWD/DMItemWidget.h \
    $$PWD/DMParamInputOrDisplay.h \
    $$PWD/DMWidget.h \
    $$PWD/DMService.h \
    $$PWD/DMUIUtility.h


SOURCES += \
    $$PWD/DMEditDialog.cpp \
    $$PWD/DMItemDataModel.cpp \
    $$PWD/DMItemWidget.cpp \
    $$PWD/DMParamInputOrDisplay.cpp \
    $$PWD/DMWidget.cpp \
    $$PWD/DMService.cpp \
    $$PWD/DMUIUtility.cpp


FORMS += \
    $$PWD/DMItemWidget.ui \
    $$PWD/DMWidget.ui

RESOURCES += \
    $$PWD/img.qrc
