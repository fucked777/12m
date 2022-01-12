INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/ConfigMacroHandler
INCLUDEPATH += $$PWD/ParamMacroHandler
INCLUDEPATH += $$PWD/OnekeyXLReleaseHandler
INCLUDEPATH += $$PWD/OnekeyXXReleaseHandler
INCLUDEPATH += $$PWD/ResourceReleaseHandler

HEADERS += \
    $$PWD/BaseHandler.h \
    $$PWD/ConfigMacroHandler/BaseLinkHandler.h \
    $$PWD/ConfigMacroHandler/ConfigMacroDispatcher.h \
    $$PWD/ConfigMacroHandler/LSYDJYXBHLinkHandler.h \
    $$PWD/ConfigMacroHandler/PKXLBPQBHLinkHandler.h \
    $$PWD/ConfigMacroHandler/RWFSLinkHandler.h \
    $$PWD/ConfigMacroHandler/SPMNYWXBHLinkHandler.h \
    $$PWD/ConfigMacroHandler/SPMNYYXBHLinkHandler.h \
    $$PWD/ConfigMacroHandler/SZHBHLinkHandler.h \
    $$PWD/ConfigMacroHandler/ZPBHLinkHandler.h \
    $$PWD/ControlFlowHandler.h \
    $$PWD/DeviceAlloter.h \
    $$PWD/OnekeyXLReleaseHandler/OnekeyXLHandler.h \
    $$PWD/OnekeyXXReleaseHandler/OnekeyXXHandler.h \
    $$PWD/ParamMacroHandler/BaseParamMacroHandler.h \
    $$PWD/ParamMacroHandler/CCZFParamMacroHandler.h \
    $$PWD/ParamMacroHandler/KaGSParamMacroHandler.h \
    $$PWD/ParamMacroHandler/ParamMacroDispatcher.h \
    $$PWD/ParamMacroHandler/STTCParamMacroHandler.h \
    $$PWD/ParamMacroHandler/TrkBBEParamMacroHandler.h \
    $$PWD/ParamMacroHandler/ACUParamMacroHandler.h \
    $$PWD/ParamMacroHandler/DSParamMacroHandler.h \
    $$PWD/ParamMacroHandler/DTETaskHandler.h \
    $$PWD/ResourceReleaseHandler/DSResourceReleaseHandler.h \
    $$PWD/ResourceReleaseHandler/BaseResourceReleaseHandler.h \
    $$PWD/ResourceReleaseHandler/KaGSResourceReleaseHandler.h \
    $$PWD/ResourceReleaseHandler/ResourceReleaseDispatcher.h \
    $$PWD/ResourceReleaseHandler/STTCResourceReleaseHandler.h \
    $$PWD/ResourceReleaseHandler/OtherResourceReleaseHandler.h \
    $$PWD/ResourceReleaseHandler/LinkReset.h \
    $$PWD/ConfigMacroHandler/GZJDHandler.h \
    $$PWD/ParamMacroHandler/UpLinkHandler.h

SOURCES += \
    $$PWD/BaseHandler.cpp \
    $$PWD/ConfigMacroHandler/BaseLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/ConfigMacroDispatcher.cpp \
    $$PWD/ConfigMacroHandler/LSYDJYXBHLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/PKXLBPQBHLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/RWFSLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/SPMNYWXBHLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/SPMNYYXBHLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/SZHBHLinkHandler.cpp \
    $$PWD/ConfigMacroHandler/ZPBHLinkHandler.cpp \
    $$PWD/ControlFlowHandler.cpp \
    $$PWD/DeviceAlloter.cpp \
    $$PWD/OnekeyXLReleaseHandler/OnekeyXLHandler.cpp \
    $$PWD/OnekeyXXReleaseHandler/OnekeyXXHandler.cpp \
    $$PWD/ParamMacroHandler/BaseParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/CCZFParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/KaGSParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/ParamMacroDispatcher.cpp \
    $$PWD/ParamMacroHandler/STTCParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/TrkBBEParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/ACUParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/DSParamMacroHandler.cpp \
    $$PWD/ParamMacroHandler/DTETaskHandler.cpp \
    $$PWD/ResourceReleaseHandler/DSResourceReleaseHandler.cpp \
    $$PWD/ResourceReleaseHandler/BaseResourceReleaseHandler.cpp \
    $$PWD/ResourceReleaseHandler/KaGSResourceReleaseHandler.cpp \
    $$PWD/ResourceReleaseHandler/ResourceReleaseDispatcher.cpp \
    $$PWD/ResourceReleaseHandler/STTCResourceReleaseHandler.cpp \
    $$PWD/ResourceReleaseHandler/OtherResourceReleaseHandler.cpp \
    $$PWD/ResourceReleaseHandler/LinkReset.cpp \
    $$PWD/ConfigMacroHandler/GZJDHandler.cpp \
    $$PWD/ParamMacroHandler/UpLinkHandler.cpp

# 校相数据库
!equals(PhaseCalibrationPri,$${TARGET}PhaseCalibrationPri){
    PhaseCalibrationPri = $${TARGET}PhaseCalibrationPri
    include($$PRIVATE_INCLUDE_DIR/PhaseCalibration/PhaseCalibrationInclude.pri)
}
