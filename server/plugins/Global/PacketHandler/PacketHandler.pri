include($$PWD/PacketHandlerInclude.pri)

HEADERS += \
    $$PWD/Packer/BasePacker.h \
    $$PWD/Packer/GroupParamSetPacker.h \
    $$PWD/Packer/ProcessControlCmdPacker.h \
    $$PWD/Packer/StatusQueryCmdPacker.h \
    $$PWD/Packer/UnitParamSetPacker.h \
    $$PWD/PacketHandler.h \
    $$PWD/ParameterHelper.h \
    $$PWD/SequenceNumberManager.h \
    $$PWD/Unpacker/BaseUnpacker.h \
    $$PWD/Unpacker/ControlCmdResponseUnpacker.h \
    $$PWD/Unpacker/ControlResultReportUnpacker.h \
    $$PWD/Unpacker/ExtensionStatusReportUnpacker.h

SOURCES += \
    $$PWD/Packer/BasePacker.cpp \
    $$PWD/Packer/GroupParamSetPacker.cpp \
    $$PWD/Packer/ProcessControlCmdPacker.cpp \
    $$PWD/Packer/StatusQueryCmdPacker.cpp \
    $$PWD/Packer/UnitParamSetPacker.cpp \
    $$PWD/PacketHandler.cpp \
    $$PWD/ParameterHelper.cpp \
    $$PWD/SequenceNumberManager.cpp \
    $$PWD/Unpacker/BaseUnpacker.cpp \
    $$PWD/Unpacker/ControlCmdResponseUnpacker.cpp \
    $$PWD/Unpacker/ControlResultReportUnpacker.cpp \
    $$PWD/Unpacker/ExtensionStatusReportUnpacker.cpp
