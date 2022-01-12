INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

!equals(PDXPPRI,$${TARGET}PDXPPRI){
    PDXPPRI = $${TARGET}PDXPPRI
    include($$PRIVATE_INCLUDE_DIR/PDXP/PDXP.pri)
}


HEADERS += \
    $$PWD/StationNetCenterCmd.h \
    $$PWD/StationNetCenterRemoteCmd.h \
    $$PWD/StationNetCenterType.h \
    $$PWD/ZwzxRemoteControlCmdHelper.h \
    $$PWD/StationNetCenterXmlReader.h \
    $$PWD/StationNetCenterMessageDefine.h

SOURCES += \
    $$PWD/StationNetCenterCmd.cpp \
    $$PWD/StationNetCenterRemoteCmd.cpp \
    $$PWD/ZwzxRemoteControlCmdHelper.cpp \
    $$PWD/StationNetCenterXmlReader.cpp
