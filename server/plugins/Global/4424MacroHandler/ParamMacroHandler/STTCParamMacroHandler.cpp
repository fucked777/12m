#include "STTCParamMacroHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "SatelliteManagementDefine.h"
#include "SingleCommandHelper.h"
STTCParamMacroHandler::STTCParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}

bool STTCParamMacroHandler::handle()
{
    SingleCommandHelper singleCommandHelper;
    PackCommand packCommand;

    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }
    // 主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    auto masterTargetWorkMode = masterTargetInfo.workMode;

    SatelliteManagementData satelliteData;
    if (!GlobalData::getSatelliteManagementData(masterTargetInfo.taskCode, satelliteData))
    {
        emit signalErrorMsg(QString("卫星代号：%1无法找到").arg(masterTargetInfo.taskCode));
        return false;
    }

    // TODO 判断当前卫星的接收极化是否和配置宏一致

    // TODO 记录当前链路的使用情况

    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);

    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }

    // 获取点频参数转换的下发结构
    QMap<int, QMap<int, QMap<QString, QVariant>>> unitTargetParamMap;
    if (!getUnitTargetParamMap(ckDeviceID, unitTargetParamMap))
    {
        return false;
    }

    // 判断当前测控基带是否为该模式，如果不是进行模式切换
    if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("%1切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
        return false;
    }
    // 根据的配置，切换测控基带主备
    switchCKJDMasterSlave();

    // 获取指定任务代号指定模式的参数宏数据
    ParamMacroModeData paramMacroModeData;
    if (!getParamMacroModeData(masterTargetTaskCode, masterTargetWorkMode, paramMacroModeData))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(masterTargetTaskCode).arg(workModeDesc));
        return false;
    }

    // 组合模式
    if (SystemWorkModeHelper::isMixWorkMode(mLinkLine.workMode))
    {
        TargetInfo noMasterTarget;
        for (auto targetNo : mLinkLine.targetMap.keys())
        {
            if (targetNo != mLinkLine.masterTargetNo)
            {
                noMasterTarget = mLinkLine.targetMap.value(targetNo);
            }
        }

        // 获取指定任务代号指定模式的参数宏数据
        ParamMacroModeData noMasterParamMacroModeData;
        if (!getParamMacroModeData(noMasterTarget.taskCode, noMasterTarget.workMode, noMasterParamMacroModeData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(masterTargetTaskCode).arg(workModeDesc));
            return false;
        }

        // 获取非主用目标设备参数，先添加到下发的参数宏中（后面再添加非主用的，这样两个共有的参数就会使用主用的）
        QMap<int, QMap<QString, QVariant>> ckjdUnitParamMap;
        noMasterParamMacroModeData.getDeviceUnitParamMap(ckDeviceID, ckjdUnitParamMap);
        appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);
    }

    // 获取测控基带设备参数，添加到下发的参数宏中
    QMap<int, QMap<QString, QVariant>> ckjdUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(ckDeviceID, ckjdUnitParamMap);
    appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);

    // 组包测控基带组参数命令
    auto modeId = SystemWorkModeHelper::systemWorkModeConverToModeId(mLinkLine.workMode);
    packGroupParamSetData(ckDeviceID, modeId, unitTargetParamMap, packCommand);
    waitExecSuccess(packCommand);

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(masterTargetTaskCode, masterTargetWorkMode, masterTargetPointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(masterTargetTaskCode, masterTargetWorkMode, masterTargetPointFreqNo, downFrequency);

    // 执行测控数字化前端组参数设置
    // 获取测控前端
    DeviceID ckqdDeviceID;
    if (!getCKQDDeviceID(ckqdDeviceID))
    {
        return false;
    }

    // 转为 QMap<单元, QMap<目标号, QMap<参数id, 值>>> 无目标的用0代表公共参数
    unitTargetParamMap.clear();
    QMap<int, QMap<QString, QVariant>> ckqdUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(ckqdDeviceID, ckqdUnitParamMap);
    for (auto unitId : ckqdUnitParamMap.keys())
    {
        unitTargetParamMap[unitId][0] = ckqdUnitParamMap[unitId];
    }
    // 替换测控前端中的频率
    unitTargetParamMap[2][0]["SendFreq_1"] = upFrequency;    // 任务的发送频率对应上行
    unitTargetParamMap[2][0]["SendFreq_2"] = downFrequency;  // 模拟源的频率对应下行

    unitTargetParamMap[3][0]["ReceiveFreq_1"] = downFrequency;  // 对应下行
    unitTargetParamMap[3][0]["ReceiveFreq_2"] = downFrequency;  // 对应下行
    unitTargetParamMap[3][0]["ReceiveFreq_3"] = upFrequency;    // 小环频率对应上行，去功放发送
    unitTargetParamMap[3][0]["ReceiveFreq_4"] = downFrequency;  // 对应下行

    packGroupParamSetData(ckqdDeviceID, 0xFFFF, unitTargetParamMap, packCommand);
    waitExecSuccess(packCommand);

    // 执行跟踪数字化前端组参数设置
    // 获取跟踪前端
    DeviceID gzqdDeviceID;
    if (!getGZQDDeviceID(gzqdDeviceID))
    {
        return false;
    }
    unitTargetParamMap.clear();
    QMap<int, QMap<QString, QVariant>> gzqdUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(gzqdDeviceID, gzqdUnitParamMap);
    for (auto unitId : gzqdUnitParamMap.keys())
    {
        unitTargetParamMap[unitId][0] = gzqdUnitParamMap[unitId];
    }
    // 替换跟踪前端中的频率
    unitTargetParamMap[2][0]["ReceiveFreq_2"] = downFrequency;  // 对应下行
    unitTargetParamMap[2][0]["ReceiveFreq_4"] = downFrequency;  // 对应下行
    unitTargetParamMap[2][0]["ReceiveFreq_6"] = downFrequency;  // 对应下行

    packGroupParamSetData(gzqdDeviceID, 0xFFFF, unitTargetParamMap, packCommand);
    waitExecSuccess(packCommand);

    // 设置S高功放发射功率
    DeviceID sggfDeviceID;
    if (!getSGGFDeviceID(sggfDeviceID))
    {
        return false;
    }
    QMap<int, QMap<QString, QVariant>> sggfUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(sggfDeviceID, sggfUnitParamMap);
    auto sggfParamMap = sggfUnitParamMap[1];
    singleCommandHelper.packSingleCommand(QString("Step_SHPA_TransmPower"), packCommand, sggfDeviceID, sggfParamMap);
    waitExecSuccess(packCommand);

    // 设置S频段射频开关网络参数
    DeviceID spdspkgwlDeviceID(3, 0, 1);
    QMap<int, QMap<QString, QVariant>> spdspkgwlUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(spdspkgwlDeviceID, spdspkgwlUnitParamMap);
    auto skgwlParamMap = spdspkgwlUnitParamMap[1];
    singleCommandHelper.packSingleCommand(QString("StepSPDSPKGWL_XHSJ"), packCommand, spdspkgwlDeviceID, skgwlParamMap);
    waitExecSuccess(packCommand);

    return true;
}

SystemWorkMode STTCParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::STTC; }
