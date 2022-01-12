QT       += core gui concurrent xml

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/Control
DEPENDPATH += $$PWD/Control

CONFIG += c++11


HEADERS += \
    $$PWD/Control/ControlFactory.h \
    $$PWD/Control/CustomIPLineEdit.h \
    $$PWD/Control/QFlowLayout.h \
    $$PWD/Control/WComboBox.h \
    $$PWD/Control/WComboBoxOnly.h \
    $$PWD/Control/WConverterWdBm.h \
    $$PWD/Control/WDateEdit.h \
    $$PWD/Control/WDateTimeEdit.h \
    $$PWD/Control/WHexLineEdit.h \
    $$PWD/Control/ControlAttribute.h \
    $$PWD/Control/WHexLineEditOnly.h \
    $$PWD/Control/WHexStringEdit.h \
    $$PWD/Control/WIPLineEdit.h \
    $$PWD/Control/WLED.h \
    $$PWD/Control/WMatrixDiagramWidget.h \
    $$PWD/Control/WMatrixLEDWidget.h \
    $$PWD/Control/WMatrixWidget.h \
    $$PWD/Control/WNumber.h \
    $$PWD/Control/WNumberOnly.h \
    $$PWD/Control/WPlainTextEdit.h \
    $$PWD/Control/WRadioButton.h \
    $$PWD/Control/WStatusLabel.h \
    $$PWD/Control/WStatusCombobox.h \
    $$PWD/Control/WStringEdit.h \
    $$PWD/Control/WTimeEdit.h \
    $$PWD/Control/WTracksNumberLineEdit.h \
    $$PWD/Control/WWidget.h \
    $$PWD/Control/XComboBox.h

SOURCES += \
    $$PWD/Control/ControlFactory.cpp \
    $$PWD/Control/CustomIPLineEdit.cpp \
    $$PWD/Control/QFlowLayout.cpp \
    $$PWD/Control/WComboBox.cpp \
    $$PWD/Control/WComboBoxOnly.cpp \
    $$PWD/Control/WConverterWdBm.cpp \
    $$PWD/Control/WDateEdit.cpp \
    $$PWD/Control/WDateTimeEdit.cpp \
    $$PWD/Control/WHexLineEdit.cpp \
    $$PWD/Control/WHexLineEditOnly.cpp \
    $$PWD/Control/WHexStringEdit.cpp \
    $$PWD/Control/WIPLineEdit.cpp \
    $$PWD/Control/WLED.cpp \
    $$PWD/Control/WMatrixDiagramWidget.cpp \
    $$PWD/Control/WMatrixLEDWidget.cpp \
    $$PWD/Control/WMatrixWidget.cpp \
    $$PWD/Control/WNumber.cpp \
    $$PWD/Control/WNumberOnly.cpp \
    $$PWD/Control/WPlainTextEdit.cpp \
    $$PWD/Control/WRadioButton.cpp \
    $$PWD/Control/WStatusLabel.cpp \
    $$PWD/Control/WStatusCombobox.cpp \
    $$PWD/Control/WStringEdit.cpp \
    $$PWD/Control/WTimeEdit.cpp \
    $$PWD/Control/WTracksNumberLineEdit.cpp \
    $$PWD/Control/WWidget.cpp \
    $$PWD/Control/XComboBox.cpp  #add wjy

FORMS +=

RESOURCES += \
    $$PWD/Control/ControlSrc.qrc


