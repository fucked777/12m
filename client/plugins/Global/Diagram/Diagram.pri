INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Item
INCLUDEPATH += $$PWD/Factory
INCLUDEPATH += $$PWD/Item/Base
INCLUDEPATH += $$PWD/Item/BaseItem
INCLUDEPATH += $$PWD/View

DEPENDPATH  += $$PWD
DEPENDPATH  += $$PWD/Item
DEPENDPATH  += $$PWD/Factory
DEPENDPATH  += $$PWD/Item/Base
DEPENDPATH  += $$PWD/Item/BaseItem
DEPENDPATH  += $$PWD/View

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}
!equals(UTILPRI,$${TARGET}UTILPRI){
    UTILPRI = $${TARGET}UTILPRI
    include($$BUSINESS_INCLUDE_DIR/Util/Util.pri)
}


HEADERS += \
    $$PWD/Factory/ArcCrossCircleFactory.h \
    $$PWD/Factory/ArrowFactory.h \
    $$PWD/Factory/CircleFactory.h \
    $$PWD/Factory/DoubleArrowFactory.h \
    $$PWD/Factory/GraphFactoryRegisterTemplate.h \
    $$PWD/Factory/GraphFactoryUtility.h \
    $$PWD/Factory/LineFactory.h \
    $$PWD/Factory/PixmapFactory.h \
    $$PWD/Factory/RectFactory.h \
    $$PWD/Factory/TextFactory.h \
    $$PWD/Factory/TwoArcCircleFactory.h \
    $$PWD/Factory/WideArrowFactory.h \
    $$PWD/Item/Base/ItemDefine.h \
    $$PWD/Item/Base/ItemUtility.h \
    $$PWD/Item/Base/ItemBase.h \
    $$PWD/Item/Base/ItemGroupBase.h \
    $$PWD/Item/BaseItem/ArcCrossCircleItem.h \
    $$PWD/Item/BaseItem/DoubleArrowItem.h \
    $$PWD/Item/BaseItem/PixmapItem.h \
    $$PWD/Item/BaseItem/WideArrowItem.h \
    $$PWD/Item/BaseItem/LineItem.h \
    $$PWD/Item/BaseItem/ArrowItem.h \
    $$PWD/Item/BaseItem/TextItem.h \
    $$PWD/Item/BaseItem/RectItem.h \
    $$PWD/Item/BaseItem/TwoArcCircleItem.h \
    $$PWD/Item/BaseItem/CircleItem.h  \
    $$PWD/Factory/GraphFactory.h \
    $$PWD/View/SystemGraphicsView.h



SOURCES += \
    $$PWD/Factory/ArcCrossCircleFactory.cpp \
    $$PWD/Factory/ArrowFactory.cpp \
    $$PWD/Factory/CircleFactory.cpp \
    $$PWD/Factory/DoubleArrowFactory.cpp \
    $$PWD/Factory/GraphFactoryUtility.cpp \
    $$PWD/Factory/LineFactory.cpp \
    $$PWD/Factory/PixmapFactory.cpp \
    $$PWD/Factory/RectFactory.cpp \
    $$PWD/Factory/TextFactory.cpp \
    $$PWD/Factory/TwoArcCircleFactory.cpp \
    $$PWD/Factory/WideArrowFactory.cpp \
    $$PWD/Item/Base/ItemUtility.cpp \
    $$PWD/Item/Base/ItemBase.cpp \
    $$PWD/Item/Base/ItemGroupBase.cpp \
    $$PWD/Item/BaseItem/ArcCrossCircleItem.cpp \
    $$PWD/Item/BaseItem/DoubleArrowItem.cpp \
    $$PWD/Item/BaseItem/LineItem.cpp \
    $$PWD/Item/BaseItem/ArrowItem.cpp \
    $$PWD/Item/BaseItem/PixmapItem.cpp \
    $$PWD/Item/BaseItem/TextItem.cpp \
    $$PWD/Item/BaseItem/RectItem.cpp \
    $$PWD/Item/BaseItem/TwoArcCircleItem.cpp \
    $$PWD/Item/BaseItem/CircleItem.cpp \
    $$PWD/Item/BaseItem/WideArrowItem.cpp \
    $$PWD/Factory/GraphFactory.cpp \
    $$PWD/View/SystemGraphicsView.cpp

QT       += network xml
QT       -= gui
QT       += widgets
