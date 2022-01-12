QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

!isEmpty(APPLICATIN_NAME){
    TARGET=$${APPLICATIN_NAME}
}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PLATFORM_INCLUDE_DIR/CppMicroServices/CppMicroServices.pri)
include($$PLATFORM_INCLUDE_DIR/QtSingleApplication/QtSingleApplication.pri)
# 没有定义此变量,证明平台配置没有引用
!equals(PLATFORMINTERFACEPRI,$${TARGET}PLATFORMINTERFACEPRI){
    PLATFORMINTERFACEPRI = $${TARGET}PLATFORMINTERFACEPRI
    include($$PLATFORM_INCLUDE_DIR/PlatformInterface/PlatformInterface.pri)
}

# 没有定义此变量,证明平台配置没有引用
#!equals(SARIBBONBAR,$${TARGET}SARIBBONBAR){
#    SARIBBONBAR = $${TARGET}SARIBBONBAR
#    include($$BUSINESS_INCLUDE_DIR/SARibbonBar/SARibbonBar.pri)
#}

win32-msvc: LIBS += -lWs2_32
DEFINES += US_BUNDLE_NAME=$$TARGET
DEFINES += MODULE_NAME=$$TARGET

DESTDIR = $$BIN_DIR

SOURCES += \
#    FrameworkWindow.cpp \
    SARibbonBar/FramelessHelper.cpp \
    SARibbonBar/SARibbonApplicationButton.cpp \
    SARibbonBar/SARibbonBar.cpp \
    SARibbonBar/SARibbonButtonGroupWidget.cpp \
    SARibbonBar/SARibbonCategory.cpp \
    SARibbonBar/SARibbonCategoryLayout.cpp \
    SARibbonBar/SARibbonCheckBox.cpp \
    SARibbonBar/SARibbonComboBox.cpp \
    SARibbonBar/SARibbonContextCategory.cpp \
    SARibbonBar/SARibbonControlButton.cpp \
    SARibbonBar/SARibbonCtrlContainer.cpp \
    SARibbonBar/SARibbonDrawHelper.cpp \
    SARibbonBar/SARibbonElementCreateDelegate.cpp \
    SARibbonBar/SARibbonElementManager.cpp \
    SARibbonBar/SARibbonGallery.cpp \
    SARibbonBar/SARibbonGalleryGroup.cpp \
    SARibbonBar/SARibbonGalleryItem.cpp \
    SARibbonBar/SARibbonLineEdit.cpp \
    SARibbonBar/SARibbonMainWindow.cpp \
    SARibbonBar/SARibbonMenu.cpp \
    SARibbonBar/SARibbonPannel.cpp \
    SARibbonBar/SARibbonPannelOptionButton.cpp \
    SARibbonBar/SARibbonQuickAccessBar.cpp \
    SARibbonBar/SARibbonSeparatorWidget.cpp \
    SARibbonBar/SARibbonStackedWidget.cpp \
    SARibbonBar/SARibbonTabBar.cpp \
    SARibbonBar/SARibbonToolButton.cpp \
    SARibbonBar/SAWindowButtonGroup.cpp \
    StartWizard.cpp \
    ModulInstall.cpp \
    ServiceInfoMap.cpp \
    MenuConfigLoad.cpp \
    PlatformConfigLoad.cpp \
    ModulConfigLoad.cpp \
    SingleModuleLoad.cpp \
    WizardLoad.cpp \
    CoreUtility.cpp \
    main.cpp \
    RibbonMainWindow.cpp

HEADERS += \
#    FrameworkWindow.h \
    SARibbonBar/FramelessHelper.h \
    SARibbonBar/SARibbonApplicationButton.h \
    SARibbonBar/SARibbonBar.h \
    SARibbonBar/SARibbonButtonGroupWidget.h \
    SARibbonBar/SARibbonCategory.h \
    SARibbonBar/SARibbonCategoryLayout.h \
    SARibbonBar/SARibbonCheckBox.h \
    SARibbonBar/SARibbonComboBox.h \
    SARibbonBar/SARibbonContextCategory.h \
    SARibbonBar/SARibbonControlButton.h \
    SARibbonBar/SARibbonCtrlContainer.h \
    SARibbonBar/SARibbonDrawHelper.h \
    SARibbonBar/SARibbonElementCreateDelegate.h \
    SARibbonBar/SARibbonElementManager.h \
    SARibbonBar/SARibbonGallery.h \
    SARibbonBar/SARibbonGalleryGroup.h \
    SARibbonBar/SARibbonGalleryItem.h \
    SARibbonBar/SARibbonGlobal.h \
    SARibbonBar/SARibbonLineEdit.h \
    SARibbonBar/SARibbonMainWindow.h \
    SARibbonBar/SARibbonMenu.h \
    SARibbonBar/SARibbonPannel.h \
    SARibbonBar/SARibbonPannelOptionButton.h \
    SARibbonBar/SARibbonQuickAccessBar.h \
    SARibbonBar/SARibbonSeparatorWidget.h \
    SARibbonBar/SARibbonStackedWidget.h \
    SARibbonBar/SARibbonTabBar.h \
    SARibbonBar/SARibbonToolButton.h \
    SARibbonBar/SAWindowButtonGroup.h \
    StartWizard.h \
    ModulInstall.h \
    ServiceInfoMap.h \
    MenuConfigLoad.h \
    PlatformConfigLoad.h \
    ModulConfigLoad.h \
    SingleModuleLoad.h \
    RibbonMainWindow.h \
    WizardLoad.h \
    CoreUtility.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32{
    TOOLS_PATH=$${FRAME_DIR}/tools/usResourceCompiler3.exe
    MODULE_NAME=$${TARGET}
    PLUG_PATH=$${FRAME_DIR}/bin/$${MODULE_NAME}.exe
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
    PLUG_PATH=$${FRAME_DIR}/bin/lib$${MODULE_NAME}
    RES_DIR=$${PWD}/resources
    RES_PATH=$${RES_DIR}/res.zip
    MANIFEST_PATH=$${RES_DIR}/manifest.json
    AFTER_LINK_CMD_LINE1 = $${TOOLS_PATH} --compression-level 9 --verbose --bundle-name $${MODULE_NAME} --out-file $${RES_PATH} --manifest-add $${MANIFEST_PATH} &&
    AFTER_LINK_CMD_LINE2 = $${TOOLS_PATH} -b $${PLUG_PATH} -z $${RES_PATH}
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE1)
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE2)
}

DISTFILES += \
    resources/manifest.json

SUBDIRS += \
    SARibbonBar/SARibbonBar.pro

RESOURCES += \
    SARibbonBar/resource.qrc
