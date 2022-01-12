INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/Controls
INCLUDEPATH += $$PWD/Controls/Base
INCLUDEPATH += $$PWD/Controls/Widgets
INCLUDEPATH += $$PWD/Module
INCLUDEPATH += $$PWD/Module/Model
INCLUDEPATH += $$PWD/Module/TaskProcessControlModule



RESOURCES += \
    $$PWD/res.qrc


!equals(QCUSTOMPLOTPRI,$${TARGET}QCUSTOMPLOTPRI){
    QCUSTOMPLOTPRI = $${TARGET}QCUSTOMPLOTPRI
    include($$PRIVATE_INCLUDE_DIR/QCustomPlot.pri)
}


HEADERS += \
    $$PWD/Controls/Base/BaseWidget.h \
    $$PWD/Controls/Base/CheckBox.h \
    $$PWD/Controls/Base/ComboBoxStatusLabel.h \
    $$PWD/Controls/Base/DynamicStatusLabel.h \
    $$PWD/Controls/Base/EnumStatusLabel.h \
    $$PWD/Controls/Base/GridLayout.h \
    $$PWD/Controls/Base/GroupBox.h \
    $$PWD/Controls/Base/LEDStatusLabel.h \
    $$PWD/Controls/Base/MultiNumberStatusLabel.h \
    $$PWD/Controls/Base/MutexCheckBox.h \
    $$PWD/Controls/Base/NameLabel.h \
    $$PWD/Controls/Base/NumberStatusLabel.h \
    $$PWD/Controls/Base/ScrollText.h \
    $$PWD/Controls/Base/StarMap.h \
    $$PWD/Controls/Base/StarMapThread.h \
    $$PWD/Controls/Base/StatusController.h \
    $$PWD/Controls/Base/StatusLabel.h \
    $$PWD/Controls/Base/StatusTimer.h \
    $$PWD/Controls/Base/VoltageRadar4424.h \
    $$PWD/Controls/Base/VoltageRadar4426.h \
    $$PWD/Controls/MainScreenControlFactory.h \
    $$PWD/Controls/Widgets/ComplexCheckBox.h \
    $$PWD/Controls/Widgets/ComplexLEDStatusLabel.h \
    $$PWD/Controls/Widgets/ComplexNumberStatusLabel.h \
    $$PWD/Controls/Widgets/DeviceTitleBar.h \
    $$PWD/Controls/Widgets/DeviceWidget.h \
    $$PWD/Controls/Widgets/DockWidget.h \
    $$PWD/Controls/Widgets/ExtensionWidget.h \
    $$PWD/Controls/Widgets/LEDStatusDecodeLabel.h \
    $$PWD/Controls/Widgets/LEDStatusStimuLabel.h \
    $$PWD/Controls/Widgets/ModeWidget.h \
    $$PWD/Controls/Widgets/NumberStatusFreqLabel.h \
    $$PWD/Controls/Widgets/NumberStatusPowerLabel.h \
    $$PWD/Controls/Widgets/NumberStatusRotationLabel.h \
    $$PWD/Module/AntennaControlWidget4424.h \
    $$PWD/Module/AntennaControlWidget4426.h \
    $$PWD/Module/AutoTaskWidget.h \
    $$PWD/Module/DeviceParamWidget.h \
    $$PWD/Module/DeviceParamWidgetGenerator.h \
    $$PWD/Module/DopplerSet.h \
    $$PWD/Module/Model/SystemLogTableModel.h \
    $$PWD/Module/Model/TaskPlanTableModel.h \
    $$PWD/Module/NavBarWidget.h \
    $$PWD/Module/RemoteWorkPeriod.h \
    $$PWD/Module/SystemLogFastPhotoWidget.h \
    $$PWD/Module/SystemLogWidget.h \
    $$PWD/Module/TaskPlanWidget.h \
    $$PWD/Module/TaskProcessControlModule/CCZFTaskPlanWidget.h \
    $$PWD/Module/TaskProcessControlModule/CKTaskWorkModeTaskCodeSelectWidget.h \
    $$PWD/Module/TaskProcessControlModule/HP.h \
    $$PWD/Module/TaskProcessControlModule/HPCmd.h \
    $$PWD/Module/TaskProcessControlModule/SCTaskWorkModeTaskCodeSelectWidget.h \
    $$PWD/Module/TaskProcessControlModule/SDTaskWorkModeTaskCodeSelectWidget.h \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessCmd.h \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessControl.h \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessXmlReader.h \
    $$PWD/Module/TaskProcessControlModule/TaskProcessControlWidget.h \
    $$PWD/Module/TimeWidget.h \
    $$PWD/Module/WorkModeXMLReader.h \
    $$PWD/StatusBarManagerThread.h \
    $$PWD/StatusBarManagerWidget.h \
    $$PWD/SystemMainScreen.h \
    $$PWD/SystemMainScreenServer.h \
    $$PWD/SystemParamMonitor.h \
    $$PWD/SystemParamWidgetContainer.h \
    $$PWD/DeviceDataLoadThread.h

SOURCES += \
    $$PWD/Controls/Base/BaseWidget.cpp \
    $$PWD/Controls/Base/CheckBox.cpp \
    $$PWD/Controls/Base/ComboBoxStatusLabel.cpp \
    $$PWD/Controls/Base/DynamicStatusLabel.cpp \
    $$PWD/Controls/Base/EnumStatusLabel.cpp \
    $$PWD/Controls/Base/GridLayout.cpp \
    $$PWD/Controls/Base/GroupBox.cpp \
    $$PWD/Controls/Base/LEDStatusLabel.cpp \
    $$PWD/Controls/Base/MultiNumberStatusLabel.cpp \
    $$PWD/Controls/Base/MutexCheckBox.cpp \
    $$PWD/Controls/Base/NameLabel.cpp \
    $$PWD/Controls/Base/NumberStatusLabel.cpp \
    $$PWD/Controls/Base/ScrollText.cpp \
    $$PWD/Controls/Base/StarMap.cpp \
    $$PWD/Controls/Base/StarMapThread.cpp \
    $$PWD/Controls/Base/StatusController.cpp \
    $$PWD/Controls/Base/StatusLabel.cpp \
    $$PWD/Controls/Base/StatusTimer.cpp \
    $$PWD/Controls/Base/VoltageRadar4424.cpp \
    $$PWD/Controls/Base/VoltageRadar4426.cpp \
    $$PWD/Controls/MainScreenControlFactory.cpp \
    $$PWD/Controls/Widgets/ComplexCheckBox.cpp \
    $$PWD/Controls/Widgets/ComplexLEDStatusLabel.cpp \
    $$PWD/Controls/Widgets/ComplexNumberStatusLabel.cpp \
    $$PWD/Controls/Widgets/DeviceTitleBar.cpp \
    $$PWD/Controls/Widgets/DeviceWidget.cpp \
    $$PWD/Controls/Widgets/DockWidget.cpp \
    $$PWD/Controls/Widgets/ExtensionWidget.cpp \
    $$PWD/Controls/Widgets/LEDStatusDecodeLabel.cpp \
    $$PWD/Controls/Widgets/LEDStatusStimuLabel.cpp \
    $$PWD/Controls/Widgets/ModeWidget.cpp \
    $$PWD/Controls/Widgets/NumberStatusFreqLabel.cpp \
    $$PWD/Controls/Widgets/NumberStatusPowerLabel.cpp \
    $$PWD/Controls/Widgets/NumberStatusRotationLabel.cpp \
    $$PWD/Module/AntennaControlWidget4424.cpp \
    $$PWD/Module/AntennaControlWidget4426.cpp \
    $$PWD/Module/AutoTaskWidget.cpp \
    $$PWD/Module/DeviceParamWidget.cpp \
    $$PWD/Module/DeviceParamWidgetGenerator.cpp \
    $$PWD/Module/DopplerSet.cpp \
    $$PWD/Module/Model/SystemLogTableModel.cpp \
    $$PWD/Module/Model/TaskPlanTableModel.cpp \
    $$PWD/Module/NavBarWidget.cpp \
    $$PWD/Module/RemoteWorkPeriod.cpp \
    $$PWD/Module/SystemLogFastPhotoWidget.cpp \
    $$PWD/Module/SystemLogWidget.cpp \
    $$PWD/Module/TaskPlanWidget.cpp \
    $$PWD/Module/TaskProcessControlModule/CCZFTaskPlanWidget.cpp \
    $$PWD/Module/TaskProcessControlModule/CKTaskWorkModeTaskCodeSelectWidget.cpp \
    $$PWD/Module/TaskProcessControlModule/HP.cpp \
    $$PWD/Module/TaskProcessControlModule/HPCmd.cpp \
    $$PWD/Module/TaskProcessControlModule/SCTaskWorkModeTaskCodeSelectWidget.cpp \
    $$PWD/Module/TaskProcessControlModule/SDTaskWorkModeTaskCodeSelectWidget.cpp \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessCmd.cpp \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessControl.cpp \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessXmlReader.cpp \
    $$PWD/Module/TaskProcessControlModule/TaskProcessControlWidget.cpp \
    $$PWD/Module/TimeWidget.cpp \
    $$PWD/Module/WorkModeXMLReader.cpp \
    $$PWD/StatusBarManagerThread.cpp \
    $$PWD/StatusBarManagerWidget.cpp \
    $$PWD/SystemMainScreen.cpp \
    $$PWD/SystemMainScreenServer.cpp \
    $$PWD/SystemParamMonitor.cpp \
    $$PWD/SystemParamWidgetContainer.cpp \
    $$PWD/DeviceDataLoadThread.cpp

FORMS += \
    $$PWD/Module/AntennaControlWidget4424.ui \
    $$PWD/Module/AntennaControlWidget4426.ui \
    $$PWD/Module/AutoTaskWidget.ui \
    $$PWD/Module/DopplerSet.ui \
    $$PWD/Module/NavBarWidget.ui \
    $$PWD/Module/RemoteWorkPeriod.ui \
    $$PWD/Module/SystemLogFastPhotoWidget.ui \
    $$PWD/Module/SystemLogWidget.ui \
    $$PWD/Module/TaskPlanWidget.ui \
    $$PWD/Module/TaskProcessControlModule/CCZFTaskPlanWidget.ui \
    $$PWD/Module/TaskProcessControlModule/CKTaskWorkModeTaskCodeSelectWidget.ui \
    $$PWD/Module/TaskProcessControlModule/HP.ui \
    $$PWD/Module/TaskProcessControlModule/SCTaskWorkModeTaskCodeSelectWidget.ui \
    $$PWD/Module/TaskProcessControlModule/SDTaskWorkModeTaskCodeSelectWidget.ui \
    $$PWD/Module/TaskProcessControlModule/SystemCaptureProcessControl.ui \
    $$PWD/Module/TaskProcessControlModule/TaskProcessControlWidget.ui \
    $$PWD/Module/TimeWidget.ui \
    $$PWD/StatusBarManagerWidget.ui \
    $$PWD/SystemMainScreen.ui

