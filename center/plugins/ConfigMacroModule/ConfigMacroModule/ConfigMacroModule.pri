INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Model


FORMS += \
    $$PWD/ConfigMacroDialog.ui \
    $$PWD/ConfigMacroManagerWidget.ui

HEADERS += \
    $$PWD/CheckBoxComboBox.h \
    $$PWD/ConfigMacroDialog.h \
    $$PWD/ConfigMacroManagerWidget.h \
    $$PWD/ConfigMacroXMLReader.h \
    $$PWD/Model/ConfigMacroTableModel.h \
    $$PWD/ConfigMacroService.h

SOURCES += \
    $$PWD/CheckBoxComboBox.cpp \
    $$PWD/ConfigMacroDialog.cpp \
    $$PWD/ConfigMacroManagerWidget.cpp \
    $$PWD/ConfigMacroXMLReader.cpp \
    $$PWD/Model/ConfigMacroTableModel.cpp \
    $$PWD/ConfigMacroService.cpp
