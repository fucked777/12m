TEMPLATE = lib


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# 微服务组件
!equals(CppMicroServicesPri,$${TARGET}CppMicroServicesPri){
    CppMicroServicesPri = $${TARGET}CppMicroServicesPri
    include($$PLATFORM_INCLUDE_DIR/CppMicroServices/CppMicroServices.pri)
}

# 平台接口
!equals(PlatformInterfacePri,$${TARGET}PlatformInterfacePri){
    PlatformInterfacePri = $${TARGET}PlatformInterfacePri
    include($$PLATFORM_INCLUDE_DIR/PlatformInterface/PlatformInterface.pri)
}

!equals(GlobalPri,$${TARGET}GlobalPri){
    GlobalPri = $${TARGET}GlobalPri
    include($$PRIVATE_INCLUDE_DIR/GlobalInclude.pri)
}

include($$PRIVATE_INCLUDE_DIR/4426MacroHandler/4426MacroHandler.pri)

DEFINES += US_BUNDLE_NAME=$$TARGET
DEFINES += MODULE_NAME=$$TARGET

DESTDIR = $$PLUGIN_DIR

SOURCES += \
    $$PWD/Activator.cpp \
    $$PWD/DistanceCalibration.cpp \
    $$PWD/SqlDistanceCalibration.cpp \
    DistanceCalibrationFlow.cpp
    

HEADERS += \
    $$PWD/DistanceCalibration.h \
    $$PWD/SqlDistanceCalibration.h \
    DistanceCalibrationFlow.h

OTHER_FILES +=     $$PWD/resources/manifest.json


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}

#add auto run bat
win32{

    TOOLS_PATH=$${FRAME_DIR}/tools/usResourceCompiler3.exe
    MODULE_NAME=$${TARGET}
    PLUG_PATH=$${FRAME_DIR}/bin/plugins/$${MODULE_NAME}.dll
    RES_DIR=$${PWD}/resources
    RES_PATH=$${RES_DIR}/res.zip
    MANIFEST_PATH=$${RES_DIR}/manifest.json
    AFTER_LINK_CMD_LINE1 = $${TOOLS_PATH} --compression-level 9 --verbose --bundle-name $${MODULE_NAME} --out-file $${RES_PATH} --manifest-add $${MANIFEST_PATH} &&
    AFTER_LINK_CMD_LINE2 = $${TOOLS_PATH} -b $${PLUG_PATH} -z $${RES_PATH}
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE1)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE2)
}


unix{

    TOOLS_PATH=$${FRAME_DIR}/tools/usResourceCompiler3
    MODULE_NAME=$${TARGET}
    PLUG_PATH=$${FRAME_DIR}/bin/plugins/lib$${MODULE_NAME}.so
    RES_DIR=$${PWD}/resources
    RES_PATH=$${RES_DIR}/res.zip
    MANIFEST_PATH=$${RES_DIR}/manifest.json
    AFTER_LINK_CMD_LINE1 = $${TOOLS_PATH} --compression-level 9 --verbose --bundle-name $${MODULE_NAME} --out-file $${RES_PATH} --manifest-add $${MANIFEST_PATH} &&
    AFTER_LINK_CMD_LINE2 = $${TOOLS_PATH} -b $${PLUG_PATH} -z $${RES_PATH}
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE1)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE2)
}
