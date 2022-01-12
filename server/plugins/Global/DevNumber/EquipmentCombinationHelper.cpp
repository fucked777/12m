#include "EquipmentCombinationHelper.h"
#include "ConfigMacroMessageDefine.h"
#include "GlobalData.h"
#include <math.h>

quint8 EquipmentCombinationHelper::getBit(quint64 sour, int indexL, int indexH)
{
    if (indexL == -1 || indexH == -1)
    {
        return 0;
    }
    quint64 temp = 2;
    while (indexH > indexL)
    {
        temp *= 2;
        indexH--;
    }
    temp -= 1;

    return static_cast<quint8>((sour >> indexL) & temp);
}

quint64 EquipmentCombinationHelper::setBit(quint64 tar, int index, bool flag)
{
    if (flag)
    {
        return tar | (quint64(1) << index);
    }
    else
    {
        return tar & ~(quint64(1) << index);
    }
}

quint64 EquipmentCombinationHelper::setBit(quint64& tar, quint8 sour, int indexL, int indexH)
{
    if (indexL == -1 || indexH == -1)
    {
        return tar;
    }

    //重置目标位段
    //    for (int i = indexL; i <= indexH; i++)
    //    {
    //        tar = setBit(tar, i, false);
    //    }
    // add wjy 按长度移位
    quint64 temp = 0;
    for (int i = 0; i <= (indexH - indexL); i++)
    {
        temp = temp | (quint64(1) << i);
    }
    sour = temp & sour;
    tar = tar | (quint64(sour) << indexL);
    return tar;
}

void EquipmentCombinationHelper::resetBit(quint64& tar, int indexL, int indexH)
{
    if (indexL == -1 || indexH == -1)
    {
        return;
    }
    for (int i = indexL; i <= indexH; i++)
    {
        tar = setBit(tar, i, false);
    }
}
bool EquipmentCombinationHelper::getCKDevNumber(SystemWorkMode systemWorkMode, const ConfigMacroData& configMacroData, quint64& devNum)
{
    //重置设备组合号为0
    devNum = 0;
    DeviceID ckDeviceID;
    if (!configMacroData.getCKJDDeviceID(systemWorkMode, ckDeviceID))
    {
        return false;
    }
    DeviceID gzDeviceID;
    if (!configMacroData.getGZJDDeviceID(systemWorkMode, gzDeviceID))
    {
        return false;
    }
    return getCKDevNumber(systemWorkMode, configMacroData, ckDeviceID, gzDeviceID, devNum);
}
bool EquipmentCombinationHelper::getCKDevNumber(SystemWorkMode systemWorkMode, const ConfigMacroData& configMacroData, const DeviceID& ckDeviceID,
                                                const DeviceID& gzDeviceID, quint64& devNum)
{
    if (!configMacroData.configMacroCmdModeMap.contains(systemWorkMode))
    {
        return false;
    }
    SystemBandMode systemBandMode = SystemWorkModeHelper::systemWorkModeToSystemBand(systemWorkMode);
    bool isCkMode = SystemWorkModeHelper::isMeasureContrlWorkMode(systemWorkMode);
    SystemBandLevel systemBandLevel = SystemWorkModeHelper::isSystemWorkModeHighOrLowBand(systemWorkMode);

    // 天线号 不是资源重组设备 天线号为0
    setAntennaNumber(devNum, 0);
    // 系统跟踪频段
    setSystemTrackingFrequencyBand(devNum, systemBandMode);
    // 设备组合号类型
    if (isCkMode)
    {
        setEquipmentCombinationType(devNum, 1);
    }
    else
    {
        setEquipmentCombinationType(devNum, 2);
    }
    if (systemBandLevel == SystemBandLevel::LowLevelBand)
    {
        // 低频段测控频段
        setLowFrequencyBandCK(devNum, systemBandMode);
    }
    else
    {
        // 高频段测控频段
        setHighFrequencyBandCK(devNum, systemBandMode);
    }

    // 上行频段
    setUplinkBand(devNum, systemBandMode);

    // 循环设置设备组合号
    auto configSetMacroCmd = configMacroData.configMacroCmdModeMap.value(systemWorkMode);
    for (auto& itemCmd : configSetMacroCmd.configMacroCmdMap)
    {
        if (0 == itemCmd.value.toInt())
        {
            continue;
            //跳过为0的情况
        }
        auto realVal = itemCmd.value.toInt() + itemCmd.offset;
        setBit(devNum, realVal, itemCmd.startBit, itemCmd.endBit);
    }
    // 更新测控基带设备组合号
    setCkDeviceNum(devNum, ckDeviceID.extenID);
    // 更新测跟踪基带设备组合号
    setGzDeviceNum(devNum, gzDeviceID.extenID);

    return true;
}
bool EquipmentCombinationHelper::getDTDevNumber(SystemWorkMode systemWorkMode, ConfigMacroData configMacroData, quint64& devNum)
{
    //重置设备组合号为0
    devNum = 0;
    DeviceID gzDeviceID;
    if (!configMacroData.getGZJDDeviceID(systemWorkMode, gzDeviceID))
    {
        return false;
    }
    return getDTDevNumber(systemWorkMode, configMacroData, gzDeviceID, devNum);
}
bool EquipmentCombinationHelper::getDTDevNumber(SystemWorkMode systemWorkMode, ConfigMacroData configMacroData, DeviceID gzDeviceID, quint64& devNum)
{
    if (!configMacroData.configMacroCmdModeMap.contains(systemWorkMode))
    {
        return false;
    }
    SystemBandMode systemBandMode = SystemWorkModeHelper::systemWorkModeToSystemBand(systemWorkMode);
    bool isCkMode = SystemWorkModeHelper::isMeasureContrlWorkMode(systemWorkMode);

    // 天线号 不是资源重组设备 天线号为0
    setAntennaNumber(devNum, 0);
    // 系统跟踪频段
    setSystemTrackingFrequencyBand(devNum, systemBandMode);
    // 设备组合号类型
    if (isCkMode)
    {
        setEquipmentCombinationType(devNum, 1);
    }
    else
    {
        setEquipmentCombinationType(devNum, 2);
    }

    // 循环设置设备组合号
    auto configSetMacroCmd = configMacroData.configMacroCmdModeMap.value(systemWorkMode);
    for (auto& itemCmd : configSetMacroCmd.configMacroCmdMap)
    {
        if (0 == itemCmd.value.toInt())
        {
            continue;
            //跳过为0的情况
        }
        auto realVal = itemCmd.value.toInt() + itemCmd.offset;
        setBit(devNum, realVal, itemCmd.startBit, itemCmd.endBit);
    }

    // 更新测跟踪基带设备组合号
    setDtGzDeviceNum(devNum, gzDeviceID.extenID);

    return true;
}

quint64 EquipmentCombinationHelper::setAntennaNumber(quint64& tar, quint8 sour) { return setBit(tar, sour, 0, 3); }
quint8 EquipmentCombinationHelper::getAntennaNumber(quint64 sour) { return getBit(sour, 0, 3); }
quint64 EquipmentCombinationHelper::setSystemTrackingFrequencyBand(quint64& tar, quint8 sour) { return setBit(tar, sour, 4, 5); }
quint8 EquipmentCombinationHelper::getSystemTrackingFrequencyBand(quint64 sour) { return getBit(sour, 4, 5); }
quint64 EquipmentCombinationHelper::setEquipmentCombinationType(quint64& tar, quint8 sour) { return setBit(tar, sour, 6, 7); }
quint8 EquipmentCombinationHelper::getEquipmentCombinationType(quint64 sour) { return getBit(sour, 6, 7); }
quint64 EquipmentCombinationHelper::setLowFrequencyBandCK(quint64& tar, quint8 sour) { return setBit(tar, sour, 8, 9); }
quint8 EquipmentCombinationHelper::getLowFrequencyBandCK(quint64 sour) { return getBit(sour, 8, 9); }

quint64 EquipmentCombinationHelper::setHighFrequencyBandCK(quint64& tar, quint8 sour) { return setBit(tar, sour, 16, 17); }

quint8 EquipmentCombinationHelper::getHighFrequencyBandCK(quint64 sour) { return getBit(sour, 16, 17); }
quint64 EquipmentCombinationHelper::setUplinkBand(quint64& tar, quint8 sour) { return setBit(tar, sour, 24, 25); }
quint8 EquipmentCombinationHelper::getUplinkBand(quint64 sour) { return getBit(sour, 24, 25); }

quint64 EquipmentCombinationHelper::setCkDeviceNum(quint64& tar, quint8 sour) { return setBit(tar, sour, 32, 35); }

quint8 EquipmentCombinationHelper::getCkDeviceNum(quint64 sour) { return getBit(sour, 32, 35); }

quint64 EquipmentCombinationHelper::setGzDeviceNum(quint64& tar, quint8 sour) { return setBit(tar, sour, 36, 39); }

quint8 EquipmentCombinationHelper::getGzDeviceNum(quint64 sour) { return getBit(sour, 36, 39); }

quint64 EquipmentCombinationHelper::setDtGzDeviceNum(quint64& tar, quint8 sour) { return setBit(tar, sour, 16, 19); }
quint8 EquipmentCombinationHelper::getDtGzDeviceNum(quint64 sour) { return getBit(sour, 16, 19); }
/************************************************************/
/* wangpeng end 20200604 */
