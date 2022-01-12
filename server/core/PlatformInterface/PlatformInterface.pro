QT       += core gui xml network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
TEMPLATE = lib
DEFINES += PLATFORMMANAGER_GLOBAL
DESTDIR = $$BIN_DIR

include($$PLATFORM_INCLUDE_DIR/CppMicroServices/CppMicroServices.pri)
# 没有定义此变量,证明JSON没有引用
#!equals(JSONPRI,$${TARGET}JSONPRI){
#    JSONPRI = $${TARGET}JSONPRI
#    include($$BUSINESS_INCLUDE_DIR/RapidJson/RapidJson.pri)
#}

HEADERS += \
    $$PWD/IGuiService.h \
    $$PWD/IWizard.h \
    $$PWD/PlatformDefine.h \
    $$PWD/PlatformConfigTools.h \
    $$PWD/PlatformInterface.h \
    $$PWD/PlatformObjectTools.h \
    $$PWD/IStartWizard.h \
    $$PWD/PlatformGlobal.h

SOURCES += \
    $$PWD/PlatformConfigTools.cpp \
    $$PWD/PlatformObjectTools.cpp \
    $$PWD/IStartWizard.cpp


win32{
    Header_File_PATH=$${PWD}
    AFTER_LINK_CMD_LINE1 = del  $${PLATFORM_INCLUDE_DIR}/PlatformInterface/*.h &&
    AFTER_LINK_CMD_LINE2 = xcopy $${Header_File_PATH}/*.h  $${PLATFORM_INCLUDE_DIR}/PlatformInterface
    AFTER_LINK_CMD_LINE1 = $$replace(AFTER_LINK_CMD_LINE1, /, \\)
    AFTER_LINK_CMD_LINE2 = $$replace(AFTER_LINK_CMD_LINE2, /, \\)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE1)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE2)
}


unix{
    Header_File_PATH=$${PWD}
    AFTER_LINK_CMD_LINE1 = rm -rf $${PLATFORM_INCLUDE_DIR}/PlatformInterface/*.h &&
    AFTER_LINK_CMD_LINE2 = cp -R $${Header_File_PATH}/*.h  $${PLATFORM_INCLUDE_DIR}/PlatformInterface
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE1)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE2)
}
