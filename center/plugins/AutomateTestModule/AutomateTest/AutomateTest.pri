QT       += network xml
QT       += gui
QT       +=widgets

include($$PRIVATE_INCLUDE_DIR/DynamicConfigToWidget/DynamicConfigToWidget.pri)

INCLUDEPATH += $$PWD

SOURCES +=  $$PWD/AutomateTestModel.cpp \
            $$PWD/AutomateTestConfig.cpp \
            $$PWD/AutomateTestResultDialog.cpp \
            $$PWD/AutomateTestUtility.cpp \
            $$PWD/AutomateTestWidget.cpp \
            $$PWD/CreateTestPlanWidget.cpp \
            $$PWD/ParameterBindingModel.cpp \
            $$PWD/TestAttributeWidget.cpp \
            $$PWD/AutomateTestResultModel.cpp \
            $$PWD/ParameterBindingSelect.cpp \
            $$PWD/ParameterBindingConfig.cpp

HEADERS +=  $$PWD/AutomateTestModel.h \
            $$PWD/AutomateTestConfig.h \
            $$PWD/AutomateTestResultDialog.h \
            $$PWD/AutomateTestUtility.h \
            $$PWD/AutomateTestWidget.h \
            $$PWD/CreateTestPlanWidget.h \
            $$PWD/ParameterBindingModel.h \
            $$PWD/TestAttributeWidget.h \
            $$PWD/AutomateTestResultModel.h \
            $$PWD/ParameterBindingSelect.h \
            $$PWD/ParameterBindingConfig.h

FORMS += \
            $$PWD/AutomateTestResultDialog.ui \
            $$PWD/AutomateTestWidget.ui \
            $$PWD/CreateTestPlanWidget.ui \
            $$PWD/ParameterBindingConfig.ui \
            $$PWD/ParameterBindingSelect.ui

RESOURCES += \
    $$PWD/AutomateTest.qrc


