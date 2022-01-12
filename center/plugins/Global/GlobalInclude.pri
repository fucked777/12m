QT       += gui widgets network printsupport xml

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
CONFIG += c++11

unix:|win32: LIBS += -L$$BIN_DIR -lGlobal
win32:!win32-g++: PRE_TARGETDEPS += $$BIN_DIR/Global.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$BIN_DIR/libGlobal.a


# 开始针对不同的公共目录不同的拷贝处理
# JSON
!equals(RapidJsonPri,$${TARGET}RapidJsonPri){
    RapidJsonPri = $${TARGET}RapidJsonPri
    include($$BUSINESS_INCLUDE_DIR/RapidJson/RapidJsonInclude.pri)
}

# 公共单元
!equals(UtilPri,$${TARGET}UtilPri){
    UtilPri = $${TARGET}UtilPri
    include($$BUSINESS_INCLUDE_DIR/Util/UtilInclude.pri)
}


# 协议解析xml
!equals(PublicTypePri,$${TARGET}PublicTypePri){
    PublicTypePri = $${TARGET}PublicTypePri
    include($$BUSINESS_INCLUDE_DIR/PublicType/PublicTypeInclude.pri)
}

# 配置宏
!equals(BusinessMacroCommonPri,$${TARGET}BusinessMacroCommonPri){
    BusinessMacroCommonPri = $${TARGET}BusinessMacroCommonPri
    include($$BUSINESS_INCLUDE_DIR/BusinessMacroCommon/BusinessMacroCommonInclude.pri)
}


# 通讯组件
!equals(CommunicationPri,$${TARGET}CommunicationPri){
    CommunicationPri = $${TARGET}CommunicationPri
    include($$BUSINESS_INCLUDE_DIR/Communication/CommunicationInclude.pri)
}

# 消息结构
!equals(MessagePri,$${TARGET}MessagePri){
    MessagePri = $${TARGET}MessagePri
    include($$BUSINESS_INCLUDE_DIR/Message/MessageInclude.pri)
}

# Redis接口
!equals(RedisCliPri,$${TARGET}RedisCliPri){
    RedisCliPri = $${TARGET}RedisCliPri
    include($$BUSINESS_INCLUDE_DIR/RedisCli/RedisCliInclude.pri)
}


# ZMQ
!equals(ZMQPri,$${TARGET}ZMQPri){
    ZMQPri = $${TARGET}ZMQPri
    include($$BUSINESS_INCLUDE_DIR/ZMQ/ZMQInclude.pri)
}


# 私有接口
!equals(InterfacePri,$${TARGET}InterfacePri){
    InterfacePri = $${TARGET}InterfacePri
    include($$PWD/Interface/InterfaceInclude.pri)
}


