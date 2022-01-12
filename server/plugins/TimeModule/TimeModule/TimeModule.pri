unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release -ltim
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -ltim



INCLUDEPATH += $$PWD



HEADERS += \
    $$PWD/NTPTime.h \
    $$PWD/NetInterfaceHelper.h \
    $$PWD/TimeInfoXMLReader.h \
    $$PWD/TimeService.h \
    $$PWD/TimeSyncThread.h \
    $$PWD/TimeSyncUDPGB.h

SOURCES += \
    $$PWD/NTPTime.cpp \
    $$PWD/NetInterfaceHelper.cpp \
    $$PWD/TimeInfoXMLReader.cpp \
    $$PWD/TimeService.cpp \
    $$PWD/TimeSyncThread.cpp \
    $$PWD/TimeSyncUDPGB.cpp

