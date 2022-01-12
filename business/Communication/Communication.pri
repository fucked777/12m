include($$PWD/CommunicationInclude.pri)

SOURCES += \
    $$PWD/INetMsg.cpp \
    $$PWD/SocketPackHelper.cpp \
    $$PWD/RecvMsgPoll.cpp

HEADERS += \
    $$PWD/INetMsg.h \
    $$PWD/PackDefine.h \
    $$PWD/SocketPackHelper.h \
    $$PWD/RecvMsgPoll.h
