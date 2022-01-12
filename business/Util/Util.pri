include($$PWD/UtilInclude.pri)
# 没有定义此变量,证明JSON没有引用
#!equals(JSONPRI,$${TARGET}JSONPRI){
#    JSONPRI = $${TARGET}JSONPRI
#    include($$BUSINESS_INCLUDE_DIR/RapidJson/RapidJson.pri)
#}

# 没有定义此变量,证明平台配置没有引用
#!equals(PLATFORMINTERFACEPRI,$${TARGET}PLATFORMINTERFACEPRI){
#    PLATFORMINTERFACEPRI = $${TARGET}PLATFORMINTERFACEPRI
#    include($$PLATFORM_INCLUDE_DIR/PlatformInterface/PlatformInterface.pri)
#}

HEADERS += \
    $$PWD/CConverter.h \
    $$PWD/CommonSelectDelegate.h \
    $$PWD/DBInterface.h \
    $$PWD/LocalCommunicationAddr.h \
    $$PWD/ErrorCode.h \
    $$PWD/ExtendedConfig.h \
    $$PWD/FileHelper.h \
    $$PWD/MacroHelper.h \
    $$PWD/QssCommonHelper.h \
    $$PWD/Singleton.h \
    $$PWD/PageNavigator.h \
    $$PWD/Utility.h \
    $$PWD/Endian.h \
    $$PWD/ServiceHelper.h


SOURCES += \
    $$PWD/CConverter.cpp \
    $$PWD/CommonSelectDelegate.cpp \
    $$PWD/DBInterface.cpp \
    $$PWD/LocalCommunicationAddr.cpp \
    $$PWD/ExtendedConfig.cpp \
    $$PWD/FileHelper.cpp \
    $$PWD/PageNavigator.cpp \
    $$PWD/Endian.cpp \
    $$PWD/QssCommonHelper.cpp \
    $$PWD/Utility.cpp

FORMS += \
    $$PWD/PageNavigator.ui

