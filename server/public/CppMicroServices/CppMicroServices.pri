win32-msvc:CONFIG(release, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Release -lCppMicroServices3
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Debug -lCppMicroServices3d
else:unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release  -lCppMicroServices
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -lCppMicroServicesd

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/cppmicroservices
DEPENDPATH += $$PWD/include/cppmicroservices

win32-msvc{
    QMAKE_CXXFLAGS += /utf-8
    # disable C4251 warning
    QMAKE_CXXFLAGS_WARN_ON += -wd4251
}

unix:!macx{
    QMAKE_LFLAGS += -Wl,--rpath,\'\$\$ORIGIN:\$\$ORIGIN/..\'
}

