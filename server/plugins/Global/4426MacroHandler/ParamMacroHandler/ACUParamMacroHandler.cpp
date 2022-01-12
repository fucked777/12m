#include "ACUParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "SatelliteManagementDefine.h"

ACUParamMacroHandler::ACUParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}

bool ACUParamMacroHandler::getTrackingMode()
{
    // 主根以测控为准
    auto result = getMasterTrackingTaskCode(mManualMsg);
    if (!result)
    {
        emit signalErrorMsg(QString("ACU参数宏下发失败:%1").arg(result.msg()));
        return false;
    }
    auto taskCode = result.value();
    SatelliteManagementData satelliteTemp;
    if (!GlobalData::getSatelliteManagementData(taskCode, satelliteTemp))
    {
        emit signalErrorMsg(QString("ACU参数宏下发失败:任务代号%1 未能查找到卫星信息").arg(taskCode));
        return false;
    }
    m_satelliteTemp = satelliteTemp;

    auto trackingInfoResult = BaseHandler::checkTrackingInfo(mManualMsg, m_satelliteTemp.m_trackingMode);
    if (!trackingInfoResult)
    {
        emit signalErrorMsg(trackingInfoResult.msg());
        return false;
    }
    m_trackingMode = std::get<0>(trackingInfoResult.value());
    m_trackingPointFreqNo = std::get<1>(trackingInfoResult.value());

    return true;
}

bool ACUParamMacroHandler::handle()
{
    auto onlineACUResult = BaseHandler::getOnlineACU();
    if (!onlineACUResult)
    {
        emit signalErrorMsg(onlineACUResult.msg());
        return false;
    }
    auto acuDeviceID = onlineACUResult.value();

    if (!getTrackingMode())
    {
        return false;
    }

    auto acuWorkMode = BaseHandler::getACUSystemWorkMode(m_trackingMode);
    if (!acuWorkMode)
    {
        emit signalErrorMsg(acuWorkMode.msg());
        return false;
    }
    auto tempWorkMode = BaseHandler::trackingToSystemWorkMode(m_trackingMode);
    if (tempWorkMode == SystemWorkMode::NotDefine)
    {
        emit signalErrorMsg("ACU参数宏获取当前主跟工作模式错误");
        return false;
    }
    /* 获取当前模式的主用点频号 */
    ParamMacroData paramMacroData;
    if (!GlobalData::getParamMacroData(m_satelliteTemp.m_satelliteCode, paramMacroData))
    {
        return false;
    }

    if (!paramMacroData.modeParamMap.contains(tempWorkMode))
    {
        auto msg = QString("获取任务代号为%1 模式为%2的参数宏数据错误")
                       .arg(SystemWorkModeHelper::systemWorkModeToDesc(tempWorkMode), m_satelliteTemp.m_satelliteCode);
        emit signalErrorMsg(msg);
        return false;
    }

    auto paramMacroModeData = paramMacroData.modeParamMap.value(tempWorkMode);

    // 获取点频和设备单元参数 并将其按单元合并到一起
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap;
    QMap<int, QVariantMap> acuTargetParamMap;
    getPointFreqAndDeviceParamMap(paramMacroModeData, m_satelliteTemp.m_satelliteCode, tempWorkMode, m_trackingPointFreqNo, acuDeviceID,
                                  acuTargetParamMap);
    unitTargetParamMap[2][0] = acuTargetParamMap[2];
    unitTargetParamMap[1][0] = acuTargetParamMap[1];

    /* 更改参数 */
    // 公共单元
    unitTargetParamMap[1][0]["SystemWorkMode"] = acuWorkMode.value();  //这个参数很重要  1:S 2:Ka遥测 3:Ka数传 4:S+Ka遥测 5:S+Ka数传
    unitTargetParamMap[1][0]["TaskIdentifier"] = m_satelliteTemp.m_satelliteIdentification;
    unitTargetParamMap[1][0]["TaskCode"] = m_satelliteTemp.m_satelliteCode;
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(tempWorkMode, mManualMsg.configMacroData, devNum);
    unitTargetParamMap[1][0]["EquipCombNumb"] = QString::number(devNum, 16);
    // ACU频率是无效的界面已经屏蔽了
    // unitTargetParamMap[1][0]["SCKUpFrequency"] = 0;
    // unitTargetParamMap[1][0]["SDownFrequency"] = 0;
    // unitTargetParamMap[1][0]["KaCKUpFrequency"] = 0;
    // unitTargetParamMap[1][0]["KaCKDownFrequency"] = 0;

    unitTargetParamMap[1][0]["AngleDataCorrect"] = 1;  //角度修正          1:修正 2:不修正 3:自动选择
    unitTargetParamMap[1][0]["AbgularProceCode"] = 1;  //测角数据标志码	1:PDXP 2:HDLC
    unitTargetParamMap[1][0]["TeleDataCode"] = 1;      // ACU 1:人工 2:自动

    /* 极化控制 PCU */
    unitTargetParamMap[6][0]["KaCKFS"] = 1;  // 1左旋 2右旋
    unitTargetParamMap[6][0]["KaSCFS"] = 1;  // 1左旋 2右旋
    unitTargetParamMap[6][0]["SFS"] = 1;     // 1左旋 2右旋

    auto findSendPolarization = [&](const QString& key, SystemWorkMode workMode) {
        auto linkLine = mManualMsg.linkLineMap.value(workMode);
        auto value = paramMacroData.modeParamMap.value(workMode);
        QVariant tempVar;
        MacroCommon::getSendPolarization(value, linkLine.targetMap.value(linkLine.masterTargetNo).pointFreqNo, tempVar);
        if (tempVar.isValid())
        {
            unitTargetParamMap[6][0][key] = tempVar;
        }
    };
    if (paramMacroData.modeParamMap.contains(KaKuo2) && mManualMsg.linkLineMap.contains(KaKuo2))
    {
        findSendPolarization("KaCKFS", KaKuo2);
    }
    if (paramMacroData.modeParamMap.contains(KaGS) && mManualMsg.linkLineMap.contains(KaGS))
    {
        findSendPolarization("KaSCFS", KaGS);
    }
    else if (paramMacroData.modeParamMap.contains(KaDS) && mManualMsg.linkLineMap.contains(KaDS))
    {
        findSendPolarization("KaSCFS", KaDS);
    }
    /* 20211112 这里认为S频段的不能同时存在 */
    if (paramMacroData.modeParamMap.contains(Skuo2) && mManualMsg.linkLineMap.contains(Skuo2))
    {
        findSendPolarization("SFS", Skuo2);
    }
    else if (paramMacroData.modeParamMap.contains(STTC) && mManualMsg.linkLineMap.contains(STTC))
    {
        findSendPolarization("SFS", STTC);
    }
    else if (paramMacroData.modeParamMap.contains(SKT) && mManualMsg.linkLineMap.contains(SKT))
    {
        findSendPolarization("SFS", SKT);
    }

    // ACU下宏只用下一台 他是控制的硬件数据是硬件上报的 所以两台机器只用下发一台就行
    packGroupParamSetData(acuDeviceID, 0xFFFF, unitTargetParamMap, mPackCommand);
    waitExecSuccess(mPackCommand);

    // 送数开
    mSingleCommandHelper.packSingleCommand("Step_ACU_2_DET_START", mPackCommand, acuDeviceID);
    waitExecSuccess(mPackCommand);

    //    if (acuDeviceID.extenID == 0x11)
    //    {
    //        acuDeviceID.extenID = 0x12;
    //    }
    //    else if (acuDeviceID.extenID == 0x12)
    //    {
    //        acuDeviceID.extenID = 0x11;
    //    }
    //    packGroupParamSetData(acuDeviceID, 0xFFFF, unitTargetParamMap, mPackCommand);
    //    waitExecSuccess(mPackCommand);

    return true;
}

SystemWorkMode ACUParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }
