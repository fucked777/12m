unix|win32: LIBS += -L$$BIN_DIR -lPlatformInterface

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
}


#HEADERS += \
#    $$PWD/IGuiService.h \
#    $$PWD/IWizard.h \
#    $$PWD/PlatformDefine.h \
#    $$PWD/PlatformConfigTools.h \
#    $$PWD/PlatformGlobal.h \
#    $$PWD/PlatformInterface.h \
#    $$PWD/PlatformThreadPool.h
