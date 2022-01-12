#include "BusinessMacroCommon.h"

#include "ConfigMacroMessageDefine.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "ResourceRestructuringMessageDefine.h"

#include <QDebug>

bool MacroCommon::getMasterCKJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(CK_BBE | Main | First);
            return true;
        }
    }
    return false;
}

bool MacroCommon::getSlaveCKJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(CK_BBE | Bak | First);
            return true;
        }
    }
    return false;
}

bool MacroCommon::getMasterGSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, BBEType gsjdType, int masterNo, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(gsjdType | Main | No(masterNo));
            return true;
        }
    }
    return false;
}

bool MacroCommon::getSlaveGSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, BBEType gsjdType, int slaveNo, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(gsjdType | Bak | No(slaveNo));
            return true;
        }
    }
    return false;
}

bool MacroCommon::getMasterDSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(DS_BBE | Main | First);
            return true;
        }
    }
    return false;
}

bool MacroCommon::getSlaveDSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            deviceID = item.getDeviceId(DS_BBE | Bak | First);
            return true;
        }
    }
    return false;
}

// bool MacroCommon::getCKQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
//{
//    // 获取该链路模式的配置宏
//    ConfigMacroData configMacroData;
//    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
//    {
//        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
//        return false;
//    }

//    if (!configMacroData.getCKQDDeviceID(workMode, deviceID))
//    {
//        qWarning() << QString("无法获取配置中测控前端的DeviceID");
//        return false;
//    }

//    return true;
//}

// bool MacroCommon::getCKQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
//{
//    // 获取该链路模式的配置宏
//    ConfigMacroData configMacroData;
//    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
//    {
//        QStringList workDescList;
//        for (auto item : configMacroWorkMode)
//        {
//            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
//        }
//        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
//        return false;
//    }

//    if (!configMacroData.getCKQDDeviceID(workMode, deviceID))
//    {
//        qWarning() << QString("无法获取配置中测控前端的DeviceID");
//        return false;
//    }

//    return true;
//}

bool MacroCommon::getGZQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getGZQDDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中跟踪前端的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getGZQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getGZQDDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中跟踪前端的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getSGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }
    //获取该配置宏里高功放的设备ID
    if (!configMacroData.getSGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中S高功放的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getSGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getSGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中S高功放的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka测控高功放的DeviceID");
        return false;
    }
    return true;
}

bool MacroCommon::getKaGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka测控高功放的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaDtGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaDtGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka数传高功放的DeviceID");
        return false;
    }
    return true;
}

bool MacroCommon::getKaDtGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaDtGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka数传高功放的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getXXBPQ(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getXXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中X下变频器的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getXXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getXXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中X下变频器的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaXBPQ(SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka下变频器的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // 获取该链路模式的配置宏
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("获取模式为%1的配置宏数据失败").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("无法获取配置中Ka下变频器的DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getUpFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("WorkRate"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
bool MacroCommon::getUpFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getUpFrequency(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}

bool MacroCommon::getDownFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SBandDownFreq"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
bool MacroCommon::getDownFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getDownFrequency(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}
bool MacroCommon::getDownFrequency(const ParamMacroData& paramMacroData, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    if (!paramMacroData.modeParamMap.contains(workMode))
    {
        return false;
    }
    auto paramMacroModeItem = paramMacroData.modeParamMap.value(workMode);

    return getDownFrequency(paramMacroModeItem, pointFreqNo, value);
}
bool MacroCommon::getFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& up, QVariant& down)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    bool isOK1 = false;
    bool isOK2 = false;
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        auto key = iter.key();
        if (key.contains("WorkRate"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            isOK1 = true;
            up = iter.value();
        }
        if (key.contains("SBandDownFreq"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            isOK2 = true;
            down = iter.value();
        }
        if (isOK1 & isOK2)
        {
            return true;
        }
    }

    return false;
}
bool MacroCommon::getFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& up, QVariant& down)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getFrequency(paramMacroModeData, pointFreqNo, up, down);
    }

    return false;
}

bool MacroCommon::getOutputLevel(const ParamMacroModeData& paramMacroModeData, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.deviceParamMap.value(deviceID).unitParamMap.value(3);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("CarrierLevel"))  // CarrierLevel
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}

bool MacroCommon::getOutputLevel(const QString& taskCode, SystemWorkMode workMode, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getOutputLevel(paramMacroModeData, value);
    }

    return false;
}

bool MacroCommon::getSendPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SEmissPolar"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
bool MacroCommon::getSendPolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getSendPolarization(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}
bool MacroCommon::getSendPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value)
{
    QVariant tempValue;
    if (getSendPolarization(paramMacroModeData, pointFreqNo, tempValue))
    {
        value = SystemOrientation(tempValue.toInt());
        return true;
    }
    value = SystemOrientation::Unkonwn;
    return false;
}
bool MacroCommon::getRecvPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SReceivPolar"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
bool MacroCommon::getRecvPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value)
{
    QVariant tempValue;
    if (getRecvPolarization(paramMacroModeData, pointFreqNo, tempValue))
    {
        value = SystemOrientation(tempValue.toInt());
        return true;
    }
    value = SystemOrientation::Unkonwn;
    return false;
}
// 从参数宏中获取指定模式下指定点频的接收极化方式(旋向) 现在都是1左旋 2右旋
bool MacroCommon::getRecvPolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getRecvPolarization(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}

bool MacroCommon::getTracePolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("STrackPolar"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
bool MacroCommon::getTracePolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value)
{
    QVariant tempValue;
    if (getTracePolarization(paramMacroModeData, pointFreqNo, tempValue))
    {
        value = SystemOrientation(tempValue.toInt());
        return true;
    }
    value = SystemOrientation::Unkonwn;
    return false;
}
// 从参数宏中获取指定模式下指定点频的跟踪极化方式(旋向) 现在都是1左旋 2右旋
bool MacroCommon::getTracePolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getTracePolarization(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}

bool MacroCommon::getAngularSamplRate(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value)
{
    ParamMacroModeData paramMacroModeData;
    if (GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        return getAngularSamplRate(paramMacroModeData, pointFreqNo, value);
    }

    return false;
}

bool MacroCommon::getAngularSamplRate(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // 配置文件需要配置 AAAA_1 表示该值是通用的
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("AngularSamplRate"))  // 有多目标的是 ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
