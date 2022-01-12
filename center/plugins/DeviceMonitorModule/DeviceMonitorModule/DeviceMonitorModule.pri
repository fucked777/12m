HEADERS += \
    $$PWD/DataExchangeSystemWidget.h \
    $$PWD/DeviceAutoMapDealWith/DeviceAutoMapDealWith.h \
    $$PWD/DeviceMonitor.h \
    $$PWD/ModelAndDelegate/CenterDataControlModel.h \
    $$PWD/ModelAndDelegate/CommunicateStatusModel.h \
    $$PWD/ModelAndDelegate/DataExchangeWidgetDelegate.h \
    $$PWD/ModelAndDelegate/HostInfoModel.h \
    $$PWD/ModelAndDelegate/MissionInfoModel.h \
    $$PWD/ModelAndDelegate/NetCardModel.h \
    $$PWD/ModelAndDelegate/SoftUnitHostModel.h \
    $$PWD/ModelAndDelegate/SoftUnitModel.h \
    $$PWD/ModelAndDelegate/TaskRunBaseNumModel.h \
    $$PWD/ModelAndDelegate/TaskRunCenterDataModel.h \
    $$PWD/ModelAndDelegate/TcpStatusModel.h \
    $$PWD/ParamRelationManager/ParamRelationManager.h \
    $$PWD/ParamRelationManager/ParamRelationOtherManager.h \
    $$PWD/SystemMonitor4424.h \
    $$PWD/SystemMonitor4426.h \
    $$PWD/WidgetDealWith/WidgetDealWith.h


SOURCES += \
    $$PWD/DataExchangeSystemWidget.cpp \
    $$PWD/DeviceAutoMapDealWith/DeviceAutoMapDealWith.cpp \
    $$PWD/DeviceMonitor.cpp \
    $$PWD/ModelAndDelegate/CenterDataControlModel.cpp \
    $$PWD/ModelAndDelegate/CommunicateStatusModel.cpp \
    $$PWD/ModelAndDelegate/DataExchangeWidgetDelegate.cpp \
    $$PWD/ModelAndDelegate/HostInfoModel.cpp \
    $$PWD/ModelAndDelegate/MissionInfoModel.cpp \
    $$PWD/ModelAndDelegate/NetCardModel.cpp \
    $$PWD/ModelAndDelegate/SoftUnitHostModel.cpp \
    $$PWD/ModelAndDelegate/SoftUnitModel.cpp \
    $$PWD/ModelAndDelegate/TaskRunBaseNumModel.cpp \
    $$PWD/ModelAndDelegate/TaskRunCenterDataModel.cpp \
    $$PWD/ModelAndDelegate/TcpStatusModel.cpp \
    $$PWD/ParamRelationManager/ParamRelationManager.cpp \
    $$PWD/ParamRelationManager/ParamRelationOtherManager.cpp \
    $$PWD/SystemMonitor4424.cpp \
    $$PWD/SystemMonitor4426.cpp \
    $$PWD/WidgetDealWith/WidgetDealWith.cpp

FORMS += \
    $$PWD/DataExchangeSystemWidget.ui \
    $$PWD/DeviceMonitor.ui \
    $$PWD/SystemMonitor4424.ui \
    $$PWD/SystemMonitor4426.ui

INCLUDEPATH += $$PWD/ModelAndDelegate \
INCLUDEPATH += $$PWD/ParamRelationManager \
INCLUDEPATH += $$PWD/WidgetDealWith
INCLUDEPATH += $$PWD/DeviceAutoMapDealWith
