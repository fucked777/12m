#include "ZwzxRemoteControlCmdHelper.h"
#include "GlobalData.h"

Optional<QVariant> ZwzxRemoteControlCmdHelper::getMACBBZValue(const DeviceID& deviceID, SystemWorkMode workMode, int unit, const QString& key)
{
    using ResType = Optional<QVariant>;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        return ResType(ErrorCode::DevOffline, "当前基带离线");
    }
    auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    int needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);

    /* 模式不一致 */
    if (curModeId != needModeId)
    {
        return ResType(ErrorCode::OperatingModeAbnormal, "基带当前工作模式与需要的工作模式不符");
    }

    auto value = GlobalData::getReportParamData(deviceID, unit, key);
    return ResType(value);
}
Optional<QVariant> ZwzxRemoteControlCmdHelper::getMACBKPValue(const DeviceID& deviceID, SystemWorkMode workMode, int target, int unit,
                                                              const QString& key)
{
    using ResType = Optional<QVariant>;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        return ResType(ErrorCode::DevOffline, "当前基带离线");
    }
    auto curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    int needModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);

    /* 模式不一致 */
    if (curModeId != needModeId)
    {
        return ResType(ErrorCode::OperatingModeAbnormal, "基带当前工作模式与需要的工作模式不符");
    }

    auto value = GlobalData::getReportParamData(deviceID, unit, key, target);
    return ResType(value);
}

/* 获取测控基带当前目标数 */
Optional<int> ZwzxRemoteControlCmdHelper::getMACBCurTargetNum(const DeviceID& deviceID, SystemWorkMode workMode)
{
    /* 虽然多目标在扩频的模式但是 这个参数是非多目标的参数 */
    using ResType = Optional<int>;
    auto result = getMACBBZValue(deviceID, workMode, 0x01, "TargetNum");
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    auto num = result->toInt();
    if (num > 0 && num <= 4)
    {
        return ResType(num);
    }

    return ResType(ErrorCode::InvalidData, "获取当前目标数错误");
}
Optional<int> ZwzxRemoteControlCmdHelper::getMACBOutputLev(const DeviceID& deviceID, SystemWorkMode workMode)
{
    using ResType = Optional<int>;
    auto result = getMACBBZValue(deviceID, workMode, 0x04, "CarrierLevel");
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    return ResType(result->toInt());
}

Optional<bool> ZwzxRemoteControlCmdHelper::getMACBKPJT(const DeviceID& deviceID, SystemWorkMode workMode, int target, const QString& key)
{
    using ResType = Optional<bool>;
    auto result = getMACBKPValue(deviceID, workMode, target, 0x04, key);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    /* 1加调 2 去调 */
    return ResType(result->toInt() == 1);
}
Optional<bool> ZwzxRemoteControlCmdHelper::getMACBBZJT(const DeviceID& deviceID, SystemWorkMode workMode, const QString& key)
{
    using ResType = Optional<bool>;
    auto result = getMACBBZValue(deviceID, workMode, 0x04, key);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    /* 1加调 2 去调 */
    return ResType(result->toInt() == 1);
}
Optional<bool> ZwzxRemoteControlCmdHelper::getMACBYKTDSCKZ(const DeviceID& deviceID, SystemWorkMode workMode, int target)
{
    using ResType = Optional<bool>;
    auto result = getMACBKPValue(deviceID, workMode, target, 0x04, "YC_ChannelCtrl");
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    /* 1允许 2禁止 */
    return ResType(result->toInt() == 1);
}
Optional<bool> ZwzxRemoteControlCmdHelper::getMACBCLTDSCKZ(const DeviceID& deviceID, SystemWorkMode workMode, int target)
{
    using ResType = Optional<bool>;
    auto result = getMACBKPValue(deviceID, workMode, target, 0x04, "CL_ChannelCtrl");
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    /* 1允许 2禁止 */
    return ResType(result->toInt() == 1);
}

Optional<bool> ZwzxRemoteControlCmdHelper::getMACBUpCarrOutput(const DeviceID& deviceID, SystemWorkMode workMode)
{
    using ResType = Optional<bool>;
    auto result = getMACBBZValue(deviceID, workMode, 0x04, "ChanCarrCtrl");
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    /* 1允许 2禁止 */
    return ResType(result->toInt() == 1);
}

Optional<DeviceID> ZwzxRemoteControlCmdHelper::getOnlineDTE()
{
    using ResType = Optional<DeviceID>;
    DeviceID deviceID;
    deviceID.sysID = 0x08;
    deviceID.devID = 0x00;
    deviceID.extenID = 0x01;

    if (GlobalData::getDeviceOnline(deviceID))
    {
        return ResType(deviceID);
    }

    deviceID.extenID = 0x02;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        return ResType(deviceID);
    }

    return ResType(ErrorCode::NotFound, "未查找当前在线的DTE");
}
