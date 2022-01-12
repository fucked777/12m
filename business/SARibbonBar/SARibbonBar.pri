INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32-msvc:CONFIG(release, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Release -lSARibbonBar
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$FRAME_DIR/lib/Win/Debug -lSARibbonBar
else:unix:!macx:CONFIG(release, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Release  -lSARibbonBar
else:unix:!macx:CONFIG(debug, debug|release):: LIBS += -L$$FRAME_DIR/lib/Linux/Debug -lSARibbonBar

