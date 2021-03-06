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
//    // ?????????????????????????????????
//    ConfigMacroData configMacroData;
//    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
//    {
//        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
//        return false;
//    }

//    if (!configMacroData.getCKQDDeviceID(workMode, deviceID))
//    {
//        qWarning() << QString("????????????????????????????????????DeviceID");
//        return false;
//    }

//    return true;
//}

// bool MacroCommon::getCKQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
//{
//    // ?????????????????????????????????
//    ConfigMacroData configMacroData;
//    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
//    {
//        QStringList workDescList;
//        for (auto item : configMacroWorkMode)
//        {
//            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
//        }
//        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
//        return false;
//    }

//    if (!configMacroData.getCKQDDeviceID(workMode, deviceID))
//    {
//        qWarning() << QString("????????????????????????????????????DeviceID");
//        return false;
//    }

//    return true;
//}

bool MacroCommon::getGZQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getGZQDDeviceID(workMode, deviceID))
    {
        qWarning() << QString("????????????????????????????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getGZQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getGZQDDeviceID(workMode, deviceID))
    {
        qWarning() << QString("????????????????????????????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getSGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }
    //???????????????????????????????????????ID
    if (!configMacroData.getSGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????S????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getSGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getSGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????S????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka??????????????????DeviceID");
        return false;
    }
    return true;
}

bool MacroCommon::getKaGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka??????????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaDtGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaDtGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka??????????????????DeviceID");
        return false;
    }
    return true;
}

bool MacroCommon::getKaDtGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaDtGGFDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka??????????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getXXBPQ(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getXXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????X???????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getXXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getXXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????X???????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaXBPQ(SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        qWarning() << QString("???????????????%1????????????????????????").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        return false;
    }

    if (!configMacroData.getKaXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka???????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getKaXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID)
{
    // ?????????????????????????????????
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(configMacroWorkMode, configMacroData))
    {
        QStringList workDescList;
        for (auto item : configMacroWorkMode)
        {
            workDescList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        qWarning() << QString("???????????????%1????????????????????????").arg(workDescList.join("+"));
        return false;
    }

    if (!configMacroData.getKaXBPQDeviceID(workMode, deviceID))
    {
        qWarning() << QString("?????????????????????Ka???????????????DeviceID");
        return false;
    }

    return true;
}

bool MacroCommon::getUpFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value)
{
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("WorkRate"))  // ?????????????????? ???_XXX  K2_WorkRate
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SBandDownFreq"))  // ?????????????????? ???_XXX  K2_WorkRate
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    bool isOK1 = false;
    bool isOK2 = false;
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        auto key = iter.key();
        if (key.contains("WorkRate"))  // ?????????????????? ???_XXX  K2_WorkRate
        {
            isOK1 = true;
            up = iter.value();
        }
        if (key.contains("SBandDownFreq"))  // ?????????????????? ???_XXX  K2_WorkRate
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SEmissPolar"))  // ?????????????????? ???_XXX  K2_WorkRate
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("SReceivPolar"))  // ?????????????????? ???_XXX  K2_WorkRate
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
// ?????????????????????????????????????????????????????????????????????(??????) ????????????1?????? 2??????
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("STrackPolar"))  // ?????????????????? ???_XXX  K2_WorkRate
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
// ?????????????????????????????????????????????????????????????????????(??????) ????????????1?????? 2??????
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
    DeviceID deviceID(0xAAAA);  // ???????????????????????? AAAA_1 ????????????????????????
    auto tempMapValue = paramMacroModeData.pointFreqParamMap.value(pointFreqNo).value(deviceID).unitParamMap.value(1);
    for (auto iter = tempMapValue.begin(); iter != tempMapValue.end(); ++iter)
    {
        if (iter.key().contains("AngularSamplRate"))  // ?????????????????? ???_XXX  K2_WorkRate
        {
            value = iter.value();
            return true;
        }
    }

    return false;
}
