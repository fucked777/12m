QT       += core gui network printsupport xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
CONFIG += c++11

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

DESTDIR = $$BIN_DIR
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += US_BUNDLE_NAME=$$TARGET
DEFINES += MODULE_NAME=$$TARGET


# JSON
!equals(RapidJsonPri,$${TARGET}RapidJsonPri){
    RapidJsonPri = $${TARGET}RapidJsonPri
    include($$BUSINESS_INCLUDE_DIR/RapidJson/RapidJson.pri)
}

# 公共单元
!equals(UtilPri,$${TARGET}UtilPri){
    UtilPri = $${TARGET}UtilPri
    include($$BUSINESS_INCLUDE_DIR/Util/Util.pri)
}


# 协议解析xml
!equals(PublicTypePri,$${TARGET}PublicTypePri){
    PublicTypePri = $${TARGET}PublicTypePri
    include($$BUSINESS_INCLUDE_DIR/PublicType/PublicType.pri)
}
# 配置宏
!equals(BusinessMacroCommonPri,$${TARGET}BusinessMacroCommonPri){
    BusinessMacroCommonPri = $${TARGET}BusinessMacroCommonPri
    include($$BUSINESS_INCLUDE_DIR/BusinessMacroCommon/BusinessMacroCommon.pri)
}


# 通讯组件
!equals(CommunicationPri,$${TARGET}CommunicationPri){
    CommunicationPri = $${TARGET}CommunicationPri
    include($$BUSINESS_INCLUDE_DIR/Communication/Communication.pri)
}

# 消息结构
!equals(MessagePri,$${TARGET}MessagePri){
    MessagePri = $${TARGET}MessagePri
    include($$BUSINESS_INCLUDE_DIR/Message/Message.pri)
}

# Redis接口
!equals(RedisCliPri,$${TARGET}RedisCliPri){
    RedisCliPri = $${TARGET}RedisCliPri
    include($$BUSINESS_INCLUDE_DIR/RedisCli/RedisCli.pri)
}


# ZMQ
!equals(ZMQPri,$${TARGET}ZMQPri){
    ZMQPri = $${TARGET}ZMQPri
    include($$BUSINESS_INCLUDE_DIR/ZMQ/ZMQ.pri)
}

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


# 私有接口
!equals(InterfacePri,$${TARGET}InterfacePri){
    InterfacePri = $${TARGET}InterfacePri
    include($$PWD/Interface/Interface.pri)
}
