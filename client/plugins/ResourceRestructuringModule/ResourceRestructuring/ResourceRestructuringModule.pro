
QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN

TARGET = ResourceRestructuringModule
TEMPLATE = lib

include($$PWD/../lxtssp/lxtssp.pri)
include($$PWD/ResourceRestructuringModule.pri)

DEFINES += RESOURCERESTRUCTURINGMODULESHARED_LIBRARY

SOURCES += ResourceRestructuringModule.cpp \
    ResourceRestructuringModuleExport.cpp

HEADERS +=  ResourceRestructuringModule.h\
            ResourceRestructuringModule_global.h


