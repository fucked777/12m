INCLUDEPATH += $$PWD


HEADERS += \
    $$PWD/AutoTask4424Service.h

SOURCES += \
    $$PWD/AutoTask4424Service.cpp


include($$PRIVATE_INCLUDE_DIR/4424MacroHandler/4424MacroHandler.pri)

include($$PRIVATE_INCLUDE_DIR/PacketHandler/PacketHandler.pri)
include($$PRIVATE_INCLUDE_DIR/LinkConfigHelper/LinkConfigHelper.pri)
include($$PRIVATE_INCLUDE_DIR/SingleCommandHelper/SingleCommandHelper.pri)

!equals(BUSINESSMACROCOMMONPRI,$${TARGET}BUSINESSMACROCOMMONPRI){
    BUSINESSMACROCOMMONPRI = $${TARGET}BUSINESSMACROCOMMONPRI
    include($$BUSINESS_INCLUDE_DIR/BusinessMacroCommon/BusinessMacroCommon.pri)
}
