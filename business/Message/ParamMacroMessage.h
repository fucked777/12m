#ifndef PARAMMACROMESSAGE_H
#define PARAMMACROMESSAGE_H

#include "DeviceProcessMessageDefine.h"
#include "JsonHelper.h"

struct DeviceData
{
    // QMap<单元id, QMap<参数id, 参数值>>
    QMap<int, QVariantMap> unitParamMap;

    // QMap<动态参数组id, QMap<单元id, QMap<第几组参数, QMap<参数id, 参数值>>>>
    QMap<QString, QMap<int, QMap<int, QVariantMap>>> dynamicParamMap;
};

struct ParamMacroModeData
{
    QMap<int, QMap<DeviceID, DeviceData>> pointFreqParamMap;  // 点频参数 QMap<点频号, QMap<设备ID, 设备参数>>
    QMap<DeviceID, DeviceData> deviceParamMap;                // 设备参数 QMap<设备ID, 设备数据>

    // 获取点频设备的单元参数
    inline bool getPointFreqUnitParamMap(int pointFreqNo, const DeviceID& deviceID, QMap<int, QVariantMap>& unitParamMap) const
    {
        if (pointFreqParamMap.contains(pointFreqNo) && pointFreqParamMap.value(pointFreqNo).contains(deviceID))
        {
            unitParamMap = pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap;
            return true;
        }
        return false;
    }

    // 获取设备的全部单元参数
    inline bool getDeviceUnitParamMap(const DeviceID& deviceID, QMap<int, QVariantMap>& unitParamMap) const
    {
        if (deviceParamMap.contains(deviceID))
        {
            unitParamMap = deviceParamMap.value(deviceID).unitParamMap;
            return true;
        }
        return false;
    }

    // 获取设备的指定单元参数
    inline bool getDeviceUnitParamMap(const DeviceID& deviceID, int unitId, QVariantMap& paramMap) const
    {
        QMap<int, QMap<QString, QVariant>> unitParamMap;
        if (!getDeviceUnitParamMap(deviceID, unitParamMap))
        {
            return false;
        }

        if (!unitParamMap.contains(unitId))
        {
            return false;
        }
        paramMap = unitParamMap.value(unitId);
        return true;
    }

    // 获取动态参数
    inline bool getDynamicParamMap(int pointFreqNo, const DeviceID& deviceID, QMap<QString, QMap<int, QMap<int, QVariantMap>>>& dynamicParamMap) const
    {
        if (pointFreqParamMap.contains(pointFreqNo) && pointFreqParamMap.value(pointFreqNo).contains(deviceID))
        {
            dynamicParamMap = pointFreqParamMap.value(pointFreqNo).value(deviceID).dynamicParamMap;
            return true;
        }

        return false;
    }
};

struct ParamMacroData
{
    QString name;
    QString taskCode;
    QString desc;
    QString workMode;  // 1+2+3 1:STTC 2:SKuo2 3:kaKuo2
    QMap<SystemWorkMode, ParamMacroModeData> modeParamMap;
};

using ParamMacroDataList = QList<ParamMacroData>;
using ParamMacroDataMap = QMap<QString, ParamMacroData>;

#endif  // CONFIGMACROMESSAGE_H
