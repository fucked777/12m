INCLUDEPATH += $$PWD


HEADERS += \
    $$PWD/AutoTask4426Service.h

SOURCES += \
    $$PWD/AutoTask4426Service.cpp


include($$PRIVATE_INCLUDE_DIR/4426MacroHandler/4426MacroHandler.pri)

include($$PRIVATE_INCLUDE_DIR/PacketHandler/PacketHandler.pri)
include($$PRIVATE_INCLUDE_DIR/LinkConfigHelper/LinkConfigHelper.pri)
include($$PRIVATE_INCLUDE_DIR/SingleCommandHelper/SingleCommandHelper.pri)

!equals(BUSINESSMACROCOMMONPRI,$${TARGET}BUSINESSMACROCOMMONPRI){
    BUSINESSMACROCOMMONPRI = $${TARGET}BUSINESSMACROCOMMONPRI
    include($$BUSINESS_INCLUDE_DIR/BusinessMacroCommon/BusinessMacroCommon.pri)
}
