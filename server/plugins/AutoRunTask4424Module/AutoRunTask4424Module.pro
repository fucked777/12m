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


include($$BUSINESS_INCLUDE_DIR/TaskPlan/TaskPlan.pri)
include($$PRIVATE_INCLUDE_DIR/4424MacroHandler/4424MacroHandler.pri)

DEFINES += US_BUNDLE_NAME=$$TARGET
DEFINES += MODULE_NAME=$$TARGET

DESTDIR = $$PLUGIN_DIR

INCLUDEPATH += machine/ \
        work/ \
        common/ \
        work/ZeroCorrectionHandler/

SOURCES += \
    $$PWD/Activator.cpp \
    $$PWD/machine/BaseEvent.cpp \
    $$PWD/machine/TaskMachine.cpp \
    $$PWD/machine/TaskScheduling.cpp \
    $$PWD/work/TaskGlobalInfo.cpp \
    $$PWD/work/TurnToWaitingPoint.cpp \
    $$PWD/work/CaptureTrackIng.cpp \
    $$PWD/work/Start.cpp \
    $$PWD/work/TaskPreparation.cpp \
    $$PWD/work/TaskStart.cpp \
    $$PWD/work/ZeroCorrection.cpp \
    $$PWD/work/Calibration.cpp \
    $$PWD/work/GoUp.cpp \
    $$PWD/work/TrackEnd.cpp \
    $$PWD/work/PostProcessing.cpp \
    $$PWD/common/TaskRunCommonHelper.cpp \
    $$PWD/work/ZeroCorrectionHandler/BaseZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/ZeroCalibrationRoutor.cpp \
    $$PWD/work/ZeroCorrectionHandler/GML_K2_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/GML_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/K2_BGZB_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/K2_GZB_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/KT_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/KaKuo2ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/SKuo2ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/SMJ_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/SttcZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/WX_K2_ZeroCalibrationHandler.cpp \
    $$PWD/work/ZeroCorrectionHandler/WX_ZeroCalibrationHandler.cpp \
    AutoRunTask4424Service.cpp

HEADERS += \
    $$PWD/machine/BaseStatus.h \
    $$PWD/machine/BaseEvent.h \
    $$PWD/machine/TaskMachine.h \
    $$PWD/machine/TaskScheduling.h \
    $$PWD/work/TaskGlobalInfo.h \
    $$PWD/work/TurnToWaitingPoint.h \
    $$PWD/work/CaptureTrackIng.h \
    $$PWD/work/Start.h \
    $$PWD/work/TaskPreparation.h \
    $$PWD/work/TaskStart.h \
    $$PWD/work/ZeroCorrection.h \
    $$PWD/work/Calibration.h \
    $$PWD/work/GoUp.h \
    $$PWD/work/TrackEnd.h \
    $$PWD/work/PostProcessing.h \
    $$PWD/common/TaskRunCommonHelper.h \
    $$PWD/work/ZeroCorrectionHandler/BaseZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/ZeroCalibrationRoutor.h \
    $$PWD/work/ZeroCorrectionHandler/GML_K2_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/GML_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/K2_BGZB_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/K2_GZB_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/KT_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/KaKuo2ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/SKuo2ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/SMJ_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/SttcZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/WX_K2_ZeroCalibrationHandler.h \
    $$PWD/work/ZeroCorrectionHandler/WX_ZeroCalibrationHandler.h \
    AutoRunTask4424Service.h

OTHER_FILES += \
    $$PWD/resources/manifest.json


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
