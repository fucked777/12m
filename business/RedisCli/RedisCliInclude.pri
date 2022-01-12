INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/tacopie
DEPENDPATH += $$PWD/tacopie
INCLUDEPATH += $$PWD/cpp_redis
DEPENDPATH += $$PWD/cpp_redis

QT       += core xml


win32-msvc:CONFIG(release, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Release -lcpp_redis
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Debug -lcpp_redisd
else:unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release -lcpp_redis
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -lcpp_redis

win32-msvc:CONFIG(release, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Release -ltacopie
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Debug -ltacopied
else:unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release -ltacopie
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -ltacopie


win32-msvc:CONFIG(release, debug|release): PRE_TARGETDEPS += $$FRAME_DIR/lib/Win/Release/cpp_redis.lib
else:win32-msvc:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$FRAME_DIR/lib/Win/Debug/cpp_redisd.lib
else:unix:!macx:CONFIG(release, debug|release):: PRE_TARGETDEPS += $$FRAME_DIR/lib/Linux/Release/libcpp_redis.a
else:unix:!macx:CONFIG(debug, debug|release):: PRE_TARGETDEPS += $$FRAME_DIR/lib/Linux/Debug/libcpp_redis.a

win32-msvc:CONFIG(release, debug|release): PRE_TARGETDEPS += $$FRAME_DIR/lib/Win/Release/tacopie.lib
else:win32-msvc:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$FRAME_DIR/lib/Win/Debug/tacopied.lib
else:unix:!macx:CONFIG(release, debug|release):: PRE_TARGETDEPS += $$FRAME_DIR/lib/Linux/Release/libtacopie.a
else:unix:!macx:CONFIG(debug, debug|release):: PRE_TARGETDEPS += $$FRAME_DIR/lib/Linux/Debug/libtacopie.a