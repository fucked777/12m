#include "OnekeyXXHandler.h"

#include "AutorunPolicyMessageSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "PKXLBPQBHLinkHandler.h"
#include "PhaseCalibrationSerialize.h"
#include "RedisHelper.h"
#include "SatelliteManagementDefine.h"
#include "UpLinkHandler.h"
#include "Utility.h"
#include <QApplication>
#include "BusinessMacroCommon.h"

class OnekeyXXHandlerImpl
{
public:
    SatelliteTrackingMode trackingMode = SatelliteTrackingMode::TrackUnknown; /* 跟踪模式 */

    PhaseCalibrationCorrMeth phaseCorrMeth = PhaseCalibrationCorrMeth::Satellite;  // 校相方式 2为当前 3是对星

    int sCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int sStandSensit = 500;                                                           /* 定向灵敏度标准值(mv) */
    int sDirectToler = 50;                                                            /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck sFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */

    int kaCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int kaStandSensit = 4800;                                                          /* 定向灵敏度标准值(mv) */
    int kaDirectToler = 400;                                                           /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck kaFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */

    int kadtCrossLowLimit = 5;                                                           /* 交叉耦合下限值(分母) */
    int kadtStandSensit = 4800;                                                          /* 定向灵敏度标准值(mv) */
    int kadtDirectToler = 400;                                                           /* 定向灵敏度允差(mv) */
    PhaseCalibrationFollowCheck kadtFollowCheck = PhaseCalibrationFollowCheck::NotCheck; /* 自跟踪检查 */

public:
    void refresh();
};
void OnekeyXXHandlerImpl::refresh()
{
    /* 自动化运行策略数据获取 */
    AutorunPolicyData autorunPolicyData;
    GlobalData::getAutorunPolicyData(autorunPolicyData);

    sCrossLowLimit = autorunPolicyData.policy.value("S_CrossLowLimit", 5).toInt();
    sStandSensit = autorunPolicyData.policy.value("S_StandSensit", 500).toInt();
    sDirectToler = autorunPolicyData.policy.value("S_DirectToler", 50).toInt();

    auto value = static_cast<int>(PhaseCalibrationFollowCheck::NotCheck);
    sFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("S_FollowCheck", value).toInt());

    kaCrossLowLimit = autorunPolicyData.policy.value("KA_CrossLowLimit", 5).toInt();
    kaStandSensit = autorunPolicyData.policy.value("KA_StandSensit", 4800).toInt();
    kaDirectToler = autorunPolicyData.policy.value("KA_DirectToler", 400).toInt();
    kaFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("KA_FollowCheck", value).toInt());

    kadtCrossLowLimit = autorunPolicyData.policy.value("KADT_CrossLowLimit", 5).toInt();
    kadtStandSensit = autorunPolicyData.policy.value("KADT_StandSensit", 4800).toInt();
    kadtDirectToler = autorunPolicyData.policy.value("KADT_DirectToler", 400).toInt();
    kadtFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("KADT_FollowCheck", value).toInt());

    value = static_cast<int>(PhaseCalibrationCorrMeth::Satellite);
    phaseCorrMeth = PhaseCalibrationCorrMeth(autorunPolicyData.policy.value("PCaM", value).toInt());
}

OnekeyXXHandler::OnekeyXXHandler(QObject* parent)
    : BaseHandler(parent)
    , m_impl(new OnekeyXXHandlerImpl)
{
}
OnekeyXXHandler::~OnekeyXXHandler() {}

bool OnekeyXXHandler::handle()
{
    emit signalSpecificTipsMsg("校相开始");

    m_impl->refresh();

    /* 1.获取当前跟踪模式 */
    mManualMsg.configMacroData.getTrackMode(m_impl->trackingMode);
    if (m_impl->trackingMode == SatelliteTrackingMode::TrackUnknown)
    {
        calibrationError("获取当前跟踪模式错误");
        return false;
    }
    //当前校相模式
    auto currentFreqBand = ACUPhaseCalibrationFreqBand::Unknown;
    if (mManualMsg.xxMode == ACUPhaseCalibrationFreqBand::Unknown)
    {
        currentFreqBand = trackingModeToAcuFreqBandStatus(m_impl->trackingMode);
    }
    else
    {
        currentFreqBand = mManualMsg.xxMode;
    }

    /* 2.获取ACU */
    //获取基带的在线状态和本控状态
    auto onlineACUResult = getOnlineACU();
    if (!onlineACUResult)
    {
        calibrationError(onlineACUResult.msg());
        return false;
    }
    auto onlineACU = onlineACUResult.value();

    // ACU快速校相(当测控基带为标准TTC或者S扩频时天线设置为S频段，Ka扩频时设置为Ka频段)
    QVariantMap acuCalibrationParamMap;
    addXuanXiang(acuCalibrationParamMap);                                               /* 旋向 */
    acuCalibrationParamMap["TaskIdent"] = mManualMsg.masterTaskBz;                      /* 任务标识 */
    acuCalibrationParamMap["PhaseCtrl"] = 1;                                            // 1开始 2结束
    acuCalibrationParamMap["EquipNumb"] = 0;                                            // 设备组合号 快速校相ACU没有使用
    acuCalibrationParamMap["FreqBand"] = static_cast<int>(currentFreqBand);             // 频段
    acuCalibrationParamMap["PhaseCorrMeth"] = static_cast<int>(m_impl->phaseCorrMeth);  //校相方式
    /* 多模式情况下以第一个校相的模式为准设置参数 */
    if (currentFreqBand == ACUPhaseCalibrationFreqBand::S || currentFreqBand == ACUPhaseCalibrationFreqBand::SKaYC ||
        currentFreqBand == ACUPhaseCalibrationFreqBand::SKaSC || currentFreqBand == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_impl->sFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_impl->sCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_impl->sStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_impl->sDirectToler);
    }
    else if (currentFreqBand == ACUPhaseCalibrationFreqBand::KaYC)
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_impl->kaFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_impl->kaCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_impl->kaStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_impl->kaDirectToler);
    }
    else
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_impl->kadtFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_impl->kadtCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_impl->kadtStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_impl->kadtDirectToler);
    }

    mSingleCommandHelper.packSingleCommand("Step_ACU_QUICK_CALIBRATION_START", mPackCommand, onlineACU, acuCalibrationParamMap);
    waitExecSuccess(mPackCommand);

    int notRecvIndex = 0;                                  /* 没有接收到数据计数 */
    QSet<ACUPhaseCalibrationFreqBand> startCalibPhaseMap;  /*开始校相频段 */
    QSet<ACUPhaseCalibrationFreqBand> finishCalibPhaseMap; /*完成校相频段 */
    QString lastHint;                                      /* 保存上一次的提示信息 保证提示信息不重复打印 */
    QString lastCheckHint;                                 /* 保存上一次的提示信息 保证提示信息不重复打印 */
    /* 休眠100毫秒 */
    auto recvTimeOut = GlobalData::getCmdTimeCount(30);   /* 时间之内数据未达到要求就超时 */
    auto timeCount = GlobalData::getCmdTimeCount(90);     /* 总超时时间 */
    int startCalibNum = 0; /* 开始校相的次数，如果连续上报过多的此时 就任务校相没有开始直接报错 */
    /* 中间的缓存数据 */
    CalibResult lastresult;
    CmdResult result;
    ACUPhaseCalibrationFreqBand successMode = ACUPhaseCalibrationFreqBand::Unknown;

    auto phaseCalibrationResultFunc = [&](SystemWorkMode workMode, ACUPhaseCalibrationFreqBand freq, CalibResultInfo& info) {
        if (mManualMsg.configMacroData.configMacroCmdModeMap.contains(workMode) && lastresult.freqBand == freq)
        {
            info.workMode = workMode;
            info.taskCode = mManualMsg.masterTaskCode;
            auto targetInfo = mManualMsg.linkLineMap.value(workMode);
            auto masterTargetNo = targetInfo.masterTargetNo <= 0 ? 1 : targetInfo.masterTargetNo;
            info.dotDrequency = targetInfo.targetMap.value(masterTargetNo).pointFreqNo;
            info.dotDrequency = info.dotDrequency <= 0 ? 1 : info.dotDrequency;
            /* 设备组合号用途 基带体制不同所以装订的参数不能是一样的 这里以设备组合号区分 */
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
            {
                EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMsg.configMacroData, info.equipComb);
            }
            else
            {
                EquipmentCombinationHelper::getDTDevNumber(workMode, mManualMsg.configMacroData, info.equipComb);
            }
            QVariant downFreq;
            MacroCommon::getDownFrequency(info.taskCode, info.workMode, info.dotDrequency, downFreq);
            info.downFreq = downFreq.toDouble();
        }
    };
    auto currentFreqBandDesc = PhaseCalibrationHelper::toString(currentFreqBand);
    auto pcSplistFB = PhaseCalibrationHelper::split(currentFreqBand);
    auto cacheKey = DeviceProcessHelper::getCmdResultRedisKey(mPackCommand.deviceID, mPackCommand.cmdId);
    for (auto i = 0; i < timeCount; i++)
    {
        if(isExit())
        {
            return false;
        }
        GlobalData::waitCmdTimeMS();
        QString json;
        // 校相数据有效期10s
        // 获取失败 数据为空
        if (!RedisHelper::getInstance().getData(cacheKey, json) || json.isEmpty())
        {
            if (notRecvIndex > recvTimeOut)
            {
                auto msg = QString("超时未收到数据，%1快速校相失败").arg(currentFreqBandDesc);
                calibrationError(msg);
                break;
            }
            notRecvIndex++;
            continue;
        }
        notRecvIndex = 0;

        json >> result;
        lastresult.freqBand = ACUPhaseCalibrationFreqBand(result.m_paramdataMap.value("FreqBand").toInt());        /* 频段 d*/
        lastresult.calibPhase = PhaseCalibrationStatus(result.m_paramdataMap.value("CalibPhase").toInt());         /* 校相状态 */
        lastresult.selfTrackResult = SelfTrackCheckResult(result.m_paramdataMap.value("SelfTrackResult").toInt()); /* 自跟踪检查结果  */
        lastresult.azimTrackZero = result.m_paramdataMap.value("AzimTrackZero").toInt();                           /* 方位跟踪零点 */
        lastresult.pitchTrackZero = result.m_paramdataMap.value("PitchTrackZero").toInt();                         /* 俯仰跟踪零点 */
        lastresult.azCorrDiffer = result.m_paramdataMap.value("AzCorrDiffer").toInt();                             /* 方位光电差修正值 */
        lastresult.pitchCorrDiff = result.m_paramdataMap.value("PitchCorrDiff").toInt();                           /* 俯仰光电差修正值 */
        lastresult.directResult = result.m_paramdataMap.value("DirectResult").toInt();                             /* 定向灵敏检查结果 */
        lastresult.azimOrient = result.m_paramdataMap.value("AzimOrient").toInt();                                 /* 方位定向灵敏度 */
        lastresult.pitchOrient = result.m_paramdataMap.value("PitchOrient").toInt();                               /* 俯仰定向灵敏度 */
        lastresult.standDireSensit = result.m_paramdataMap.value("StandDireSensit").toInt();                       /* 定向灵敏度标准值 */
        lastresult.crossExamResu = result.m_paramdataMap.value("CrossExamResu").toInt();                           /* 交叉耦合检查结果 */
        lastresult.azimCrossCoup = result.m_paramdataMap.value("AzimCrossCoup").toInt();                           /* 方位交叉耦合  */
        lastresult.pitchCrossCoup = result.m_paramdataMap.value("PitchCrossCoup").toInt();                         /* 俯仰交叉耦合 */
        lastresult.crossLowerLim = result.m_paramdataMap.value("CrossLowerLim").toInt();                           /* 交叉耦合下限值(分母) */
        lastresult.azpr = result.m_paramdataMap.value("AZPR").toDouble();                                          /* 方位校相结果 */
        lastresult.elpr = result.m_paramdataMap.value("ELPR").toDouble();                                          /* 俯仰校相结果 */
        lastresult.azge = result.m_paramdataMap.value("AZGE").toDouble();                                          /* 方位移相值 */
        lastresult.elge = result.m_paramdataMap.value("ELGE").toDouble();                                          /* 俯仰移相值 */

        auto phaseCalibrationStatusDesc = PhaseCalibrationHelper::toString(lastresult.calibPhase);
        if (lastresult.calibPhase == PhaseCalibrationStatus::Error ||                              //
            lastresult.calibPhase == PhaseCalibrationStatus::NotFoundTrack ||                      //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandQueryTimeout ||               //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandPhaseCalibrationTimeout ||    //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandBindingTimeout ||             //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandRejectionPhaseCalibration ||  //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandRejectionBinding ||           //
            lastresult.calibPhase == PhaseCalibrationStatus::ReceiverOutOfLock ||                  //
            lastresult.calibPhase == PhaseCalibrationStatus::AGCAbnormal ||                        //
            lastresult.calibPhase == PhaseCalibrationStatus::ErrorVoltageOverrun ||                //
            lastresult.calibPhase == PhaseCalibrationStatus::BasebandBindingFailed)

        {
            calibrationError(phaseCalibrationStatusDesc);
            break;
        }

        if (lastresult.calibPhase == PhaseCalibrationStatus::Start)
        {
            startCalibPhaseMap << lastresult.freqBand;
            startCalibNum++;
        }
        else
        {
            startCalibNum = 0;
        }

        if (startCalibNum > 120)
        {
            calibrationError("ACU无法进行快速校相，校相终止");
            break;
        }

        if (lastresult.calibPhase == PhaseCalibrationStatus::Finish)
        {
            finishCalibPhaseMap << lastresult.freqBand;
            CalibResultInfo info;
            info.azpr = lastresult.azpr;
            info.elpr = lastresult.elpr;
            info.azge = lastresult.azge;
            info.elge = lastresult.elge;

            phaseCalibrationResultFunc(STTC, ACUPhaseCalibrationFreqBand::S, info);
            phaseCalibrationResultFunc(Skuo2, ACUPhaseCalibrationFreqBand::S, info);
            phaseCalibrationResultFunc(KaKuo2, ACUPhaseCalibrationFreqBand::KaYC, info);
            phaseCalibrationResultFunc(KaDS, ACUPhaseCalibrationFreqBand::KaSC, info);
            phaseCalibrationResultFunc(KaGS, ACUPhaseCalibrationFreqBand::KaSC, info);

            if (lastresult.freqBand != successMode)
            {
                BaseHandler::updateCalibResultInfoList(info);
                successMode = lastresult.freqBand;

                auto msg = QString("%1校相成功 ACU当前方位校相%2,俯仰校相结果%3,方位移相值%4,俯仰移相值%5")
                               .arg(SystemWorkModeHelper::systemWorkModeToDesc(info.workMode))
                               .arg(lastresult.azge)
                               .arg(lastresult.elge)
                               .arg(lastresult.azpr)
                               .arg(lastresult.elpr);
                emit signalSpecificTipsMsg(msg);
            }
        }

        /* 自跟踪检查 定向灵敏度和交叉耦合检查 */
        /* 自跟踪检查 */
        // if (lastresult.calibPhase == PhaseCalibrationStatus::FinishSelfTrackingCheck)
        // {
        //     auto msg = PhaseCalibrationHelper::toString(lastresult.selfTrackResult);
        //     if (!(lastresult.selfTrackResult == SelfTrackCheckResult::NormalPolarity ||  //
        //           lastresult.selfTrackResult == SelfTrackCheckResult::NoCheck))
        //     {
        //         calibrationError(QString("自跟踪检查结果:%1").arg(msg));
        //         break;
        //     }
        //     else
        //     {
        //         emit signalInfoMsg(QString("自跟踪检查结果:%1").arg(msg));
        //     }
        // }

        // /* 交叉耦合 */
        // if (lastresult.calibPhase == PhaseCalibrationStatus::FinishOSAndCCCheck)
        // {
        //     /* 1 是通过 2是不通过 */
        //     if (lastresult.directResult == 2)
        //     {
        //         emit signalErrorMsg(QString("定向灵敏度检查结果:未通过"));
        //     }
        //     else
        //     {
        //         emit signalInfoMsg(QString("定向灵敏度检查结果:通过"));
        //     }

        //     if (lastresult.crossExamResu == 2)
        //     {
        //         emit signalErrorMsg(QString("交叉耦合检查结果:未通过"));
        //     }
        //     else
        //     {
        //         emit signalInfoMsg(QString("交叉耦合检查结果:未通过"));
        //     }
        // }

        if (lastresult.calibPhase == PhaseCalibrationStatus::FinishSelfTrackingCheck ||  //
            lastresult.calibPhase == PhaseCalibrationStatus::FinishOSAndCCCheck)
        {
            auto msg = PhaseCalibrationHelper::toString(lastresult.selfTrackResult);
            if (lastCheckHint != msg)
            {
                emit signalInfoMsg(msg);
                lastCheckHint = msg;
            }

            if (!(lastresult.selfTrackResult == SelfTrackCheckResult::NormalPolarity ||  //
                  lastresult.selfTrackResult == SelfTrackCheckResult::NoCheck))
            {
                calibrationError("校相终止");
                break;
            }
        }
        if (phaseCalibrationStatusDesc != lastHint)
        {
            auto msg = QString("%1 ACU当前方位校相%2,俯仰校相结果%3,方位移相值%4,俯仰移相值%5")
                           .arg(phaseCalibrationStatusDesc)
                           .arg(lastresult.azge)
                           .arg(lastresult.elge)
                           .arg(lastresult.azpr)
                           .arg(lastresult.elpr);
            emit signalInfoMsg(msg);
            lastHint = phaseCalibrationStatusDesc;
        }

        /* 成功与否检查 */
        int successCount = 0;
        for (auto& item : pcSplistFB)
        {
            successCount += static_cast<int>(finishCalibPhaseMap.contains(item));
        }
        if (successCount == pcSplistFB.size())
        {
            emit signalSpecificTipsMsg(QString("%1快速校相完成").arg(currentFreqBandDesc));
            return true;
        }
    }

    return false;
}

void OnekeyXXHandler::calibrationError(const QString& logStr)
{
    emit signalErrorMsg(logStr);
    emit sg_audio("校相错误");
}
void OnekeyXXHandler::addXuanXiang(QVariantMap& acuCalibrationParamMap)
{
    auto getTrackPolar = [&](SystemWorkMode workMode) {
        SystemOrientation systemOrientation = SystemOrientation::Unkonwn;
        mManualMsg.configMacroData.getTrackPolar(workMode, systemOrientation);
        if (systemOrientation == SystemOrientation::Unkonwn || systemOrientation == SystemOrientation::LRCircumflex)
        {
            systemOrientation = SystemOrientation::LCircumflex;
        }
        return static_cast<int>(systemOrientation);
    };

    // 旋向
    auto sXuanxiang = getTrackPolar(STTC);
    auto kaycXuanxiang = getTrackPolar(KaKuo2);
    auto kascXuanxiang = getTrackPolar(KaGS);
    // 默认是STTC 所以只检查SKuo2 kt
    if (m_impl->trackingMode == _4426_SK2 ||              //
        m_impl->trackingMode == _4426_SK2_KaCk ||         //
        m_impl->trackingMode == _4426_SK2_KaDSDT ||       //
        m_impl->trackingMode == _4426_SK2_KaGSDT ||       //
        m_impl->trackingMode == _4426_SK2_KaCk_KaDSDT ||  //
        m_impl->trackingMode == _4426_SK2_KaCK_KaGSDT)
    {
        sXuanxiang = getTrackPolar(Skuo2);
    }
    else if (m_impl->trackingMode == _4426_SKT)
    {
        sXuanxiang = getTrackPolar(SKT);
    }

    // 默认是低速 所以只检查高速
    if (m_impl->trackingMode == _4426_KaGSDT ||            //
        m_impl->trackingMode == _4426_STTC_KaGSDT ||       //
        m_impl->trackingMode == _4426_STTC_KaCK_KaGSDT ||  //
        m_impl->trackingMode == _4426_SK2_KaGSDT ||        //
        m_impl->trackingMode == _4426_SK2_KaCK_KaGSDT)
    {
        kascXuanxiang = getTrackPolar(KaGS);
    }
    acuCalibrationParamMap["SEmissPolar"] = sXuanxiang;        // S接收极化 1：左旋 2：右旋
    acuCalibrationParamMap["KaSCEmissPolar"] = kascXuanxiang;  // Ka数传接收极化
    acuCalibrationParamMap["KaYCEmissPolar"] = kaycXuanxiang;  // Ka接收极化 1：左旋 2：右旋
}
ACUPhaseCalibrationFreqBand OnekeyXXHandler::trackingModeToAcuFreqBandStatus(SatelliteTrackingMode mode)
{
    if (mode == _4426_STTC ||  //
        mode == _4426_SK2 ||   //
        mode == _4426_SKT)
    {
        return ACUPhaseCalibrationFreqBand::S;
    }

    if (mode == _4426_KaCk)
    {
        return ACUPhaseCalibrationFreqBand::KaYC;
    }

    if (mode == _4426_KaDSDT || mode == _4426_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::KaSC;
    }

    // 截止20211023 扩跳不能快速校相
    if (mode == _4426_STTC_KaCk ||  //
        mode == _4426_SK2_KaCk)
    {
        return ACUPhaseCalibrationFreqBand::KaYC;
    }

    if (mode == _4426_STTC_KaDSDT ||  //
        mode == _4426_STTC_KaGSDT ||  //
        mode == _4426_SK2_KaDSDT ||   //
        mode == _4426_SK2_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::SKaSC;
    }

    if (mode == _4426_STTC_KaCk_KaDSDT ||  //
        mode == _4426_STTC_KaCK_KaGSDT ||  //
        mode == _4426_SK2_KaCk_KaDSDT ||   //
        mode == _4426_SK2_KaCK_KaGSDT)
    {
        return ACUPhaseCalibrationFreqBand::SKaYCKaSC;
    }
    return ACUPhaseCalibrationFreqBand::Unknown;
}
