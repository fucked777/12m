#include "DeviceAutoMapDealWith.h"
#include "WidgetDealWith.h"

DeviceAutoMapDealWith& DeviceAutoMapDealWith::instance()
{
    static DeviceAutoMapDealWith deviceAutoMap;
    return deviceAutoMap;
}

void DeviceAutoMapDealWith::decideDeviceAutoMap(DeviceID& deviceID, ModeCtrl& mode, ExtensionStatusReportMessage& statusReport)
{
    WidgetDealWith widgetDealWith;
    if (deviceID.sysID == 4 && deviceID.devID == 0 && mode.ID == 0x1)  //测控基带 标准TTC
    {
        if (statusReport.unitReportMsgMap.contains(8) && statusReport.unitReportMsgMap.contains(12))  //遥测单元数据和模拟源单元数据都存在
        {
            decideCKDataTAS(statusReport.unitReportMsgMap[8], statusReport.unitReportMsgMap[12]);
        }
    }
    else if (deviceID.sysID == 4 && deviceID.devID == 0 && mode.ID == 0x4)  //测控基带 扩频
    {
        if (statusReport.unitReportMsgMap.contains(6) && statusReport.unitReportMsgMap.contains(8))  //遥测单元数据和模拟源单元数据都存在
        {
            decideCKDataTAS(statusReport.unitReportMsgMap[6], statusReport.unitReportMsgMap[8]);
        }
    }
    else if (deviceID.sysID == 4 && deviceID.devID == 0 &&
             (mode.ID == 0x10 || mode.ID == 0x11 ||
              mode.ID == 0x12))  //测控基带 一体化上面级/卫星/上行高码率  单模式的判断放一起  后面有骚操作再加一个函数就行了
    {
        if (statusReport.unitReportMsgMap.contains(6) && statusReport.unitReportMsgMap.contains(8))  //遥测单元数据和模拟源单元数据都存在
        {
            decideCKDataTAS(statusReport.unitReportMsgMap[6], statusReport.unitReportMsgMap[8]);
        }
    }
    else if (deviceID.sysID == 4 && deviceID.devID == 0 &&
             (mode.ID == 0x13 || mode.ID == 0x15 || mode.ID == 0x17 || mode.ID == 0x19))  //测控基带 一体化混合模式  混合模式的放一起
    {
        if (statusReport.unitReportMsgMap.contains(6) && statusReport.unitReportMsgMap.contains(8))  //遥测单元数据和模拟源单元数据都存在
        {
            decideCKDataTAS(statusReport.unitReportMsgMap[6], statusReport.unitReportMsgMap[8]);
        }
    }
}

void DeviceAutoMapDealWith::decideCKDataTAS(UnitReportMessage& tuData, UnitReportMessage& tasData)
{
    for (auto tasKey : tasData.paramMap.keys())
    {
        if (tuData.paramMap.contains(tasKey))
            tasData.paramMap[tasKey] = tuData.paramMap[tasKey];
    }

    for (auto tasRecycle : tasData.multiTargetParamMap.keys())
    {
        for (auto tasKey : tasData.multiTargetParamMap[tasRecycle].keys())
        {
            if (tuData.multiTargetParamMap.contains(tasRecycle))
            {
                if (tuData.multiTargetParamMap[tasRecycle].contains(tasKey))
                    tasData.multiTargetParamMap[tasRecycle][tasKey] = tuData.multiTargetParamMap[tasRecycle][tasKey];
            }
        }
    }
}
