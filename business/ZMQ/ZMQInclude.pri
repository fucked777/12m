win32-msvc:CONFIG(release, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Release -llibzmq
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Debug -llibzmqd
else:unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release -lzmq
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -lzmq


INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
