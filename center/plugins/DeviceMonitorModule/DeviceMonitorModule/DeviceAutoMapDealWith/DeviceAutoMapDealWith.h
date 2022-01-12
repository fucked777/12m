#ifndef DEVICEAUTOMAPDEALWITH_H
#define DEVICEAUTOMAPDEALWITH_H

#include "CustomPacketMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class DeviceAutoMapDealWith
{
public:
    static DeviceAutoMapDealWith& instance();
    //判断是哪个设备哪个模式需要自动映射处理
    void decideDeviceAutoMap(DeviceID& deviceID, ModeCtrl& mode, ExtensionStatusReportMessage& statusReport);
    //测控基带模拟源单元映射   目前没有需要特殊处理的单元， 因此都用一个函数
    void decideCKDataTAS(UnitReportMessage& tuData, UnitReportMessage& tasData);
};

#endif  // DEVICEAUTOMAPDEALWITH_H
