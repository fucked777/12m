#include "Calibration.h"

#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PhaseCalibrationSerialize.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"
#include <QDebug>
#include <QThread>
Calibration::Calibration(QObject* parent)
    : BaseEvent(parent)
{
}

void Calibration::doSomething()
{
    QString uuid = taskMachine->getMachineUUID();
    m_taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    if (!m_taskTimeList.m_calibration)
    {
        notifyInfoLog(QString("当前任务配置为不校相，跳过校相流程"));
        taskMachine->updateStatus("Calibration", TaskStepStatus::Continue);
        programTaskStart();
        return;
    }

    notifyInfoLog("校相处理流程开始执行");
    VoiceAlarmPublish::publish("校相开始");
    // ACU快速校相(当测控基带为标准TTC或者S扩频时天线设置为S频段，Ka扩频时设置为Ka频段)
    QVariantMap acuCalibrationParamMap;

    //当前校相模式
    auto currentFreqBand = TaskRunCommonHelper::trackingModeToAcuFreqBandStatus(m_taskTimeList.trackingMode);
    addXuanXiang(acuCalibrationParamMap);                                    /* 旋向 */
    acuCalibrationParamMap["TaskIdent"] = m_taskTimeList.masterTaskBz;       /* 任务标识 */
    acuCalibrationParamMap["PhaseCtrl"] = 1;                                 // 1开始 2结束
    acuCalibrationParamMap["EquipNumb"] = 0;                                 // 设备组合号 快速校相ACU没有使用
    acuCalibrationParamMap["FreqBand"] = static_cast<int>(currentFreqBand);  // 频段
    acuCalibrationParamMap["PhaseCorrMeth"] = static_cast<int>(m_taskTimeList.commonTaskConfig.phaseCorrMeth);  //校相方式

    // ACU快速校相 S频段
    if (currentFreqBand == ACUPhaseCalibrationFreqBand::S)
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_taskTimeList.commonTaskConfig.sFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.sCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.sStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_taskTimeList.commonTaskConfig.sDirectToler);
    }
    else if (currentFreqBand == ACUPhaseCalibrationFreqBand::KaYC ||  //
             currentFreqBand == ACUPhaseCalibrationFreqBand::SKaYC)
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kaFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kaCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kaStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kaDirectToler);
    }
    else
    {
        acuCalibrationParamMap["FollowCheck"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kadtFollowCheck);      //自跟踪检查
        acuCalibrationParamMap["CrossLowLimit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kadtCrossLowLimit);  //交叉耦合下限值
        acuCalibrationParamMap["StandSensit"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kadtStandSensit);      //定向灵敏度标准值
        acuCalibrationParamMap["DirectToler"] = static_cast<int>(m_taskTimeList.commonTaskConfig.kadtDirectToler);
    }

    m_singleCommandHelper.packSingleCommand("Step_ACU_QUICK_CALIBRATION_START", m_packCommand, m_taskTimeList.onlineACU, acuCalibrationParamMap);
    waitExecSuccess(m_packCommand);

    int notRecvIndex = 0;                                        /* 没有接收到数据计数 */
    QMap<ACUPhaseCalibrationFreqBand, int> m_startCalibPhaseMap; /*开始校相频段 */
    QSet<ACUPhaseCalibrationFreqBand> finishCalibPhaseMap;       /*完成校相频段 */
    QString lastHint;                                            /* 保存上一次的提示信息 保证提示信息不重复打印 */
    /* 休眠100毫秒 */
    auto recvTimeOut = 30 * 10;   /* 时间之内数据未达到要求就超时 */
    auto timeCount = 3 * 60 * 10; /* 总超时时间 */
    int startCalibNum = 0;        /* 开始校相的次数，如果连续上报过多的此时 就任务校相没有开始直接报错 */
    /* 中间的缓存数据 */
    CalibResult lastresult;
    CmdResult result;
    bool isSuccess = false;

    auto phaseCalibrationResultFunc = [&](SystemWorkMode workMode, ACUPhaseCalibrationFreqBand freq, CalibResultInfo& info) {
        if (m_taskTimeList.m_dataMap.contains(workMode) && lastresult.freqBand == freq)
        {
            info.workMode = workMode;
            info.taskCode = m_taskTimeList.manualMessage.masterTaskCode;
            auto targetInfo = m_taskTimeList.manualMessage.linkLineMap.value(workMode);
            auto masterTargetNo = targetInfo.masterTargetNo<=0 ?1 : targetInfo.masterTargetNo;
            info.dotDrequency = targetInfo.targetMap.value(masterTargetNo).pointFreqNo;
            info.dotDrequency= info.dotDrequency<=0 ? 1 : info.dotDrequency;
        }
    };
    auto currentFreqBandDesc = PhaseCalibrationHelper::toString(currentFreqBand);
    auto pcSplistFB = PhaseCalibrationHelper::split(currentFreqBand);
    for (auto i = 0; i < timeCount; i++)
    {
        QThread::msleep(100);
        QString json;
        // 校相数据有效期10s
        // 获取失败 数据为空
        if (!RedisHelper::getInstance().getData("ACUCalibrationResult", json) || json.isEmpty())
        {
            if (notRecvIndex > recvTimeOut)
            {
                //如果时S遥测
                calibrationError(QString("超时未收到数据，%1快速校相失败").arg(currentFreqBandDesc));
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
            notifyErrorLog(phaseCalibrationStatusDesc);
            notifyErrorLog("校相终止");
            break;
        }

        if (lastresult.calibPhase == PhaseCalibrationStatus::Start)
        {
            m_startCalibPhaseMap[lastresult.freqBand] = true;
            startCalibNum++;
        }
        else
        {
            startCalibNum = 0;
        }

        if (startCalibNum > 90)
        {
            notifyErrorLog("ACU无法进行快速校相，校相终止");
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
            TaskGlobalInfoSingleton::getInstance().updateCalibResultInfoList(info);
        }

        /* 自跟踪检查 定向灵敏度和交叉耦合检查 */
        if (lastresult.calibPhase == PhaseCalibrationStatus::FinishSelfTrackingCheck ||  //
            lastresult.calibPhase == PhaseCalibrationStatus::FinishOSAndCCCheck)
        {
            auto msg = PhaseCalibrationHelper::toString(lastresult.selfTrackResult);
            notifyInfoLog(msg);
            if (!(lastresult.selfTrackResult == SelfTrackCheckResult::NormalPolarity ||  //
                  lastresult.selfTrackResult == SelfTrackCheckResult::NoCheck))
            {
                calibrationError("校相终止");
                break;
            }
        }
        if (phaseCalibrationStatusDesc != lastHint)
        {
            auto msg = QString("%1 ACU当前方位校相%1,俯仰校相结果%2,方位移相值%3,俯仰移相值%4")
                           .arg(phaseCalibrationStatusDesc)
                           .arg(lastresult.azge)
                           .arg(lastresult.elge)
                           .arg(lastresult.azpr)
                           .arg(lastresult.elpr);
            notifyInfoLog(msg);
            // lastHint = phaseCalibrationStatusDesc;
        }

        /* 成功与否检查 */
        int successCount = 0;
        for (auto& item : pcSplistFB)
        {
            successCount += static_cast<int>(finishCalibPhaseMap.contains(item));
        }
        if (successCount == pcSplistFB.size())
        {
            notifyInfoLog(QString("%1快速校相完成").arg(currentFreqBandDesc));
            isSuccess = true;
            break;
        }
    }
    notifyInfoLog("校相处理流程完成");
    if (isSuccess)
    {
        trackTaskStart();
    }
    else
    {
        taskMachine->updateStatus("Calibration", TaskStepStatus::Error);
        programTaskStart();
    }

    // 4413
    //    notifyInfoLog("校相处理流程开始执行");
    //    VoiceAlarmPublish::publish("校相开始");
    //    QString uuid = taskMachine->getMachineUUID();

    //    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    //    PackCommand packCommand;

    //    QList<uint> taskIndentList;
    //    for (auto workModeID : taskTimeList.m_currentXuanXiangMap.keys())
    //    {
    //        QString workModeStr;
    //        if (workModeID == 1)
    //            workModeStr = "标准TTC";
    //        if (workModeID == 2)
    //            workModeStr = "S扩二";
    //        if (workModeID == 3)
    //            workModeStr = "Ka扩二";
    //        if (workModeID == 4)
    //            workModeStr = "X低速";
    //        if (workModeID == 5)
    //            workModeStr = "Ka低速";
    //        if (workModeID == 6)
    //            workModeStr = "Ka高速";
    //        QString xuanxiangStr;
    //        if (taskTimeList.m_currentXuanXiangMap[workModeID] == 1)
    //            xuanxiangStr = "左旋";
    //        if (taskTimeList.m_currentXuanXiangMap[workModeID] == 2)
    //            xuanxiangStr = "右旋";
    //        if (taskTimeList.m_currentXuanXiangMap[workModeID] == 3)
    //            xuanxiangStr = "左右旋";

    //        notifyInfoLog(QString("工作模式（%1），极化（%2）").arg(workModeStr).arg(xuanxiangStr));
    //        for (auto value : taskTimeList.m_dataMap[workModeID])
    //        {
    //            if (value.isMaster)
    //            {
    //                notifyInfoLog(QString("任务代号%1，任务标识%2，设备组合号%3")
    //                                  .arg(value.m_taskCode)
    //                                  .arg(QString::number(value.m_taskBz, 16))
    //                                  .arg(QString::number(value.m_equipNum, 16)));
    //                taskIndentList.append(value.m_taskBz);
    //            }
    //        }
    //    }
    //    // ACU快速校相(当测控基带为标准TTC或者S扩频时天线设置为S频段，Ka扩频时设置为Ka频段)
    //    QMap<QString, QVariant> acuCalibrationParamMap;
    //    uint acuTaskIndent = 0;
    //    if (taskIndentList.size() > 0)
    //    {
    //        acuCalibrationParamMap["TaskIdent"] = taskIndentList[0];  // 任务标识
    //        acuTaskIndent = taskIndentList[0];
    //    }
    //    else
    //    {
    //        acuCalibrationParamMap["TaskIdent"] = 0xFFFF;  // 任务标识
    //        notifyErrorLog("快速校相无法查找到任务标识");
    //    }
    //    enum LinkFreqBandStatus
    //    {
    //        S = 1,
    //        KaCk = 2,
    //        KaDSDT = 3,
    //        KaGSDT = 4,
    //        S_KaCk = 5,
    //        S_KaDSDT = 6,
    //        S_KaGSDT = 7,
    //        S_KaCk_KaDSDT = 8,
    //        S_KaCK_KaGSDT = 9,
    //    };

    //    enum AcuFreqBandStatus
    //    {
    //        AcuFreqBandStatusS = 1,           // S
    //        AcuFreqBandStatusKaYC = 2,        // Ka遥测
    //        AcuFreqBandStatusKaDT = 3,        // Ka数传
    //        AcuFreqBandStatusS_KaYC = 4,      // S+Ka遥测
    //        AcuFreqBandStatusS_KaDT = 5,      // S+Ka数传
    //        AcuFreqBandStatusS_KaYC_KaDT = 6  // S+Ka遥测+Ka数传
    //    };

    //    //当前校相模式
    //    // int bxMode = 1;
    //    int currentFreqBand = 1;
    //    //标准TTC 单模式 ->S
    //    if (taskTimeList.m_dataMap.contains(STTC))
    //    {
    //        // bxMode = LinkFreqBandStatus::S;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = 1;                                         // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }
    //    //扩频TTC 单模式  ->S
    //    if (taskTimeList.m_dataMap.contains(Skuo2))
    //    {
    //        // bxMode = LinkFreqBandStatus::S;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = 1;                                          // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                         // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Kak2扩频 单模式  ->Ka遥测
    //    if (taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        // bxMode = LinkFreqBandStatus::KaCk;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        acuCalibrationParamMap["SEmissPolar"] = 1;                                            // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = 1;                                            // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka低速 单模式  ->Ka低速
    //    if (taskTimeList.m_dataMap.contains(KaDS))
    //    {
    //        // bxMode = LinkFreqBandStatus::KaDSDT;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;

    //        acuCalibrationParamMap["SEmissPolar"] = 1;                                         // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];  // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka高速 单模式  ->Ka高速
    //    if (taskTimeList.m_dataMap.contains(KaGS))
    //    {
    //        // bxMode = LinkFreqBandStatus::KaGSDT;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;

    //        acuCalibrationParamMap["SEmissPolar"] = 1;                                         // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaGS];  // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }

    //    // Ka扩二+ 标准TTC
    //    if (taskTimeList.m_dataMap.contains(KaKuo2) && (taskTimeList.m_dataMap.contains(STTC)))
    //    {
    //        // bxMode = LinkFreqBandStatus::S_KaCk;
    //        // currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];     // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = 1;                                            // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    if (taskTimeList.m_dataMap.contains(KaKuo2) && (taskTimeList.m_dataMap.contains(Skuo2)))
    //    {
    //        // bxMode = LinkFreqBandStatus::S_KaCk;
    //        // currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];    // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = 1;                                            // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // X低速 + 标准TTC
    //    if (taskTimeList.m_dataMap.contains(XDS) && (taskTimeList.m_dataMap.contains(STTC)))
    //    {
    //        // bxMode = LinkFreqBandStatus::S_KaDSDT;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[XDS];   // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }
    //    if (taskTimeList.m_dataMap.contains(XDS) && (taskTimeList.m_dataMap.contains(Skuo2)))
    //    {
    //        // bxMode = LinkFreqBandStatus::S_KaDSDT;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[XDS];    // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                         // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka低速 + 标准TTC
    //    if (taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(STTC)))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];  // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }
    //    if (taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(Skuo2)))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];   // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                         // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka高速 + 标准TTC
    //    if (taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(STTC)))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];  // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                        // Ka接收极化 1：左旋 2：右旋
    //    }
    //    if (taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(Skuo2)))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];  // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];   // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = 1;                                         // Ka接收极化 1：左旋 2：右旋
    //    }

    //    // Ka高速数传 +Ka测控
    //    if (taskTimeList.m_dataMap.contains(KaDS) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = 1;                                            // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // ka低速数传 +ka测控
    //    if (taskTimeList.m_dataMap.contains(KaDS) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = 1;                                            // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }

    //    // Ka低速 + 标准TTC + ka扩二
    //    if (taskTimeList.m_dataMap.contains(KaDS) && taskTimeList.m_dataMap.contains(STTC) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];     // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    if (taskTimeList.m_dataMap.contains(KaDS) && taskTimeList.m_dataMap.contains(Skuo2) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk_KaDSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];    // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka高速 + 标准TTC + ka扩二
    //    if (taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(STTC)) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCK_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;

    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[STTC];     // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }
    //    // Ka高速 + S扩2 +ka扩二
    //    if (taskTimeList.m_dataMap.contains(KaDS) && taskTimeList.m_dataMap.contains(Skuo2) && taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //        if (taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCK_KaGSDT)
    //            currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;
    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;
    //        acuCalibrationParamMap["SEmissPolar"] = taskTimeList.m_currentXuanXiangMap[Skuo2];    // S接收极化 1：左旋 2：右旋
    //        acuCalibrationParamMap["XEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaDS];     // Ka数传接收极化
    //        acuCalibrationParamMap["KaEmissPolar"] = taskTimeList.m_currentXuanXiangMap[KaKuo2];  // Ka接收极化 1：左旋 2：右旋
    //    }

    //    //    //Ka低速 + Ka高速 + 标准TTC + ka扩二
    //    //    if(taskTimeList.m_dataMap.contains(KaDS) && (taskTimeList.m_dataMap.contains(STTC) || taskTimeList.m_dataMap.contains(Skuo2)) &&
    //    //    taskTimeList.m_dataMap.contains(KaKuo2)){
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::S) currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS;
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::KaCk) currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaYC;
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::KaGSDT) currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusKaDT;
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCk) currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC;
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaGSDT) currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaDT;
    //    //        if(taskTimeList.m_trackingMode == LinkFreqBandStatus::S_KaCK_KaGSDT) currentFreqBand =
    //    AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;
    //    //        currentFreqBand = AcuFreqBandStatus::AcuFreqBandStatusS_KaYC_KaDT;
    //    //    }

    //    DeviceID deviceACUMaster;  //主用的ACU
    //    auto onlineACUResult = getOnlineACU();
    //    if (!onlineACUResult)
    //    {
    //        notifyErrorLog(onlineACUResult.msg());
    //        // notify("error");
    //        // return;
    //        onlineACUResult = Optional<DeviceID>(ACU_A);  // 有设备或者正常调试以后 删出 此行
    //    }

    //    deviceACUMaster = onlineACUResult.value();

    //    // ACU快速校相 S频段
    //    acuCalibrationParamMap["FreqBand"] = currentFreqBand;  // 频段 1:S频段 2：Ka频段
    //    acuCalibrationParamMap["EquipNumb"] = 0;               // 设备组合号               //快速校相ACU没有使用
    //    acuCalibrationParamMap["CrossLowLimit"] = 5;           //交叉耦合下限值
    //    acuCalibrationParamMap["StandSensit"] = 4800;          //定向灵敏度标准值
    //    acuCalibrationParamMap["DirectToler"] = 400;           //定向灵敏度允差
    //    m_singleCommandHelper.packSingleCommand("Step_AC_34_1", packCommand, deviceACUMaster, acuCalibrationParamMap);
    //    waitExecSuccess(packCommand);

    //    // 30s内处理校相数据
    //    int recvIndex = 0;
    //    int notRecvIndex = 0;

    //    QMap<int, int> m_startCalibPhaseMap;   //开始校相频段
    //    QMap<int, int> m_finishCalibPhaseMap;  //完成校相频段
    //                                           //    m_runningDeviceID = deviceACUMaster;

    //    bool isSuccess = true;
    //    int zeroNum = 0;  //校零结果为0的时候 超过5次直接退出
    //    QString lastHint;
    //    int startCalibNum = 0;  //开始校相的次数，如果连续上报10次开始校相 就任务校相没有开始直接报错
    //    for (auto i = 0; i < 180; i++)
    //    {
    //        UnpackMessage unpackMsg;
    //        CalibResult lastresult;
    //        notRecvIndex++;
    // TODO 获取redis数据
    //        if (waitExecXXResponceData(unpackMsg))
    //        {
    //            recvIndex++;
    //            notRecvIndex = 0;
    //            CalibResult result;
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("FreqBand"))
    //                result.FreqBand = unpackMsg.cmdReportMessage.reportMap["FreqBand"].toInt();  //频段
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("CalibPhase"))
    //                result.CalibPhase = unpackMsg.cmdReportMessage.reportMap["CalibPhase"].toInt();  //校相状态
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("SelfTrackResult"))
    //                result.SelfTrackResult = unpackMsg.cmdReportMessage.reportMap["SelfTrackResult"].toInt();  //自跟踪检查结果
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AzimTrackZero"))
    //                result.AzimTrackZero = unpackMsg.cmdReportMessage.reportMap["AzimTrackZero"].toInt();  //方位跟踪零点
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("PitchTrackZero"))
    //                result.PitchTrackZero = unpackMsg.cmdReportMessage.reportMap["PitchTrackZero"].toInt();  //俯仰跟踪零点
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AzCorrDiffer"))
    //                result.AzCorrDiffer = unpackMsg.cmdReportMessage.reportMap["AzCorrDiffer"].toInt();  //方位光电差修正值
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("PitchCorrDiff"))
    //                result.PitchCorrDiff = unpackMsg.cmdReportMessage.reportMap["PitchCorrDiff"].toInt();  //俯仰光电差修正值
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("DirectResult"))
    //                result.DirectResult = unpackMsg.cmdReportMessage.reportMap["DirectResult"].toInt();  //定向灵敏检查结果
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AzimOrient"))
    //                result.AzimOrient = unpackMsg.cmdReportMessage.reportMap["AzimOrient"].toInt();  //方位定向灵敏度
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("PitchOrient"))
    //                result.PitchOrient = unpackMsg.cmdReportMessage.reportMap["PitchOrient"].toInt();  //俯仰定向灵敏度
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("StandDireSensit"))
    //                result.StandDireSensit = unpackMsg.cmdReportMessage.reportMap["StandDireSensit"].toInt();  //定向灵敏度标准值
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("CrossExamResu"))
    //                result.CrossExamResu = unpackMsg.cmdReportMessage.reportMap["CrossExamResu"].toInt();  //交叉耦合检查结果
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AzimCrossCoup"))
    //                result.AzimCrossCoup = unpackMsg.cmdReportMessage.reportMap["AzimCrossCoup"].toInt();  //方位交叉耦合
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("PitchCrossCoup"))
    //                result.PitchCrossCoup = unpackMsg.cmdReportMessage.reportMap["PitchCrossCoup"].toInt();  //俯仰交叉耦合
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("CrossLowerLim"))
    //                result.CrossLowerLim = unpackMsg.cmdReportMessage.reportMap["CrossLowerLim"].toInt();  //交叉耦合下限值(分母)
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AZPR"))
    //                result.AZPR = unpackMsg.cmdReportMessage.reportMap["AZPR"].toDouble();  //方位移相值
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("ELPR"))
    //                result.ELPR = unpackMsg.cmdReportMessage.reportMap["ELPR"].toDouble();  //俯仰移相值
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("AZGE"))
    //                result.AZGE = unpackMsg.cmdReportMessage.reportMap["AZGE"].toDouble();  //方位斜率
    //            if (unpackMsg.cmdReportMessage.reportMap.contains("ELGE"))
    //                result.ELGE = unpackMsg.cmdReportMessage.reportMap["ELGE"].toDouble();  //俯仰斜率

    //            if (result.CalibPhase == CalibResult::Error || result.CalibPhase == CalibResult::NotFindGuiDao ||
    //                result.CalibPhase == CalibResult::NotFindDevice || result.CalibPhase == CalibResult::NotResponsed ||
    //                result.CalibPhase == CalibResult::NotSetCalibPhase2Device || result.CalibPhase == CalibResult::NotControlDevice ||
    //                result.CalibPhase == CalibResult::NotSendCalib2Device || result.CalibPhase == CalibResult::NotFindRecvDevice ||
    //                result.CalibPhase == CalibResult::AGCError || result.CalibPhase == CalibResult::WCDYError ||
    //                result.CalibPhase == CalibResult::NotSetDeviceAZ)
    //            {
    //                notifyErrorLog(result.getCalibPhase());
    //                notifyErrorLog("校相终止");
    //                break;
    //            }
    //            else
    //            {
    //                if (result.CalibPhase == CalibResult::StartCalibPhase)
    //                {
    //                    m_startCalibPhaseMap[result.FreqBand] = true;
    //                    startCalibNum++;
    //                }
    //                else
    //                {
    //                    startCalibNum = 0;
    //                }
    //                if (startCalibNum > 90)
    //                {
    //                    notifyErrorLog("ACU无法进行快速校相，校相终止");
    //                    break;
    //                }
    //                if (result.CalibPhase == CalibResult::FinishCalibPhase)
    //                {
    //                    m_finishCalibPhaseMap[result.FreqBand] = true;
    //                    CalibResultInfo info;
    //                    info.AZPR = result.AZPR;
    //                    info.ELPR = result.ELPR;
    //                    info.AZGE = result.AZGE;
    //                    info.ELGE = result.ELGE;

    //                    //更新S频段的
    //                    if (taskTimeList.m_dataMap.contains(STTC) && result.FreqBand == 1)
    //                    {
    //                        info.workMode = 1;
    //                        for (auto item : taskTimeList.m_dataMap[STTC])
    //                        {
    //                            if (item.isMaster)
    //                            {
    //                                info.taskCode = item.m_taskCode;
    //                                info.workFreg = taskTimeList.m_downFrequencyMap[STTC];
    //                            }
    //                        }
    //                    }
    //                    if (taskTimeList.m_dataMap.contains(Skuo2) && result.FreqBand == 1)
    //                    {
    //                        info.workMode = 2;
    //                        for (auto item : taskTimeList.m_dataMap[Skuo2])
    //                        {
    //                            if (item.isMaster)
    //                            {
    //                                info.taskCode = item.m_taskCode;
    //                                info.workFreg = taskTimeList.m_downFrequencyMap[Skuo2];
    //                            }
    //                        }
    //                    }
    //                    if (taskTimeList.m_dataMap.contains(KaKuo2) && result.FreqBand == 2)
    //                    {
    //                        info.workMode = 3;
    //                        for (auto item : taskTimeList.m_dataMap[KaKuo2])
    //                        {
    //                            if (item.isMaster)
    //                            {
    //                                info.taskCode = item.m_taskCode;
    //                                info.workFreg = taskTimeList.m_downFrequencyMap[KaKuo2];
    //                            }
    //                        }
    //                    }
    //                    if (taskTimeList.m_dataMap.contains(KaDS) && result.FreqBand == 3)
    //                    {
    //                        info.workMode = 5;
    //                        for (auto item : taskTimeList.m_dataMap[KaDS])
    //                        {
    //                            if (item.isMaster)
    //                            {
    //                                info.taskCode = item.m_taskCode;
    //                                info.workFreg = taskTimeList.m_downFrequencyMap[KaDS];
    //                            }
    //                        }
    //                    }
    //                    if (taskTimeList.m_dataMap.contains(KaDS) && result.FreqBand == 3)
    //                    {
    //                        info.workMode = 6;
    //                        for (auto item : taskTimeList.m_dataMap[KaDS])
    //                        {
    //                            if (item.isMaster)
    //                            {
    //                                info.taskCode = item.m_taskCode;
    //                                info.workFreg = taskTimeList.m_downFrequencyMap[KaDS];
    //                            }
    //                        }
    //                    }
    //                    TaskGlobalInfoSingleton::getInstance().updateCalibResultInfoList(info);
    //                }

    //                //                if((lastresult.CalibPhase == result.CalibPhase
    //                //                     && lastresult.FreqBand == result.FreqBand
    //                //                     && lastresult.AZPR == result.AZPR
    //                //                     && lastresult.ELPR == result.ELPR
    //                //                     && lastresult.AZGE == result.AZGE
    //                //                     && lastresult.ELGE == result.ELGE))
    //                //                {
    //                //                    continue;
    //                //                }
    //                //                else{
    //                //                    lastresult = result;
    //                //                }
    //                if (result.CalibPhase == CalibResult::EndZGZJC || result.CalibPhase == CalibResult::EndLMDJC)
    //                {
    //                    if (result.SelfTrackResult == CalibResult::JiXingZhengChang || result.SelfTrackResult == CalibResult::NoCheck)
    //                    {
    //                        notifyInfoLog(result.getSelfTrackResult());
    //                    }
    //                    else
    //                    {
    //                        notifyErrorLog(result.getSelfTrackResult());
    //                        notifyErrorLog("校相终止");
    //                        break;
    //                    }
    //                }
    //                //                if(result.AZGE == 0 && result.AZGE ==0 && result.AZGE == 0 && result.AZGE == 0){
    //                //                    zeroNum++;
    //                //                }else{
    //                //                    zeroNum = 0;
    //                //                }
    //                //                if(zeroNum > 5){
    //                //                    notifyErrorLog("ACU当前方位校相0,俯仰校相结果0,方位移相值0,俯仰移相值0，校相终止");
    //                //                    break;
    //                //                }
    //                if (result.getCalibPhase() != lastHint)
    //                {
    //                    notifyInfoLog(result.getCalibPhase() + QString("ACU当前方位校相%1,俯仰校相结果%2,方位移相值%3,俯仰移相值%4")
    //                                                               .arg(result.AZGE)
    //                                                               .arg(result.ELGE)
    //                                                               .arg(result.AZPR)
    //                                                               .arg(result.ELPR));
    //                    lastHint = result.getCalibPhase();
    //                }
    //            }
    //        }
    //        else
    //        {
    //            if (notRecvIndex > 30)
    //            {
    //                // 10s内没收到校相结果代表失败
    //                //如果时S遥测
    //                if (currentFreqBand == AcuFreqBandStatusS && !m_finishCalibPhaseMap.contains(1))
    //                {
    //                    notifyErrorLog("超时未收到数据，S频段快速校相失败");
    //                    break;
    //                }
    //                if (currentFreqBand == AcuFreqBandStatusKaYC && !m_finishCalibPhaseMap.contains(2))
    //                {
    //                    notifyErrorLog("超时未收到数据，Ka遥测快速校相失败");
    //                    break;
    //                }
    //                if (currentFreqBand == AcuFreqBandStatusKaDT && !m_finishCalibPhaseMap.contains(3))
    //                {
    //                    notifyErrorLog("超时未收到数据，Ka数传快速校相失败");
    //                    break;
    //                }
    //                if (currentFreqBand == AcuFreqBandStatusKaDT && (!m_finishCalibPhaseMap.contains(1) || !m_finishCalibPhaseMap.contains(3)))
    //                {
    //                    notifyErrorLog("超时未收到数据，S+Ka遥测快速校相失败");
    //                    break;
    //                }
    //                if (currentFreqBand == AcuFreqBandStatusKaDT && (!m_finishCalibPhaseMap.contains(1) || !m_finishCalibPhaseMap.contains(2)))
    //                {
    //                    notifyErrorLog("超时未收到数据，S+Ka数传快速校相失败");
    //                    break;
    //                }
    //                if (currentFreqBand == AcuFreqBandStatusKaDT &&
    //                    (!m_finishCalibPhaseMap.contains(1) || !m_finishCalibPhaseMap.contains(2) || !m_finishCalibPhaseMap.contains(3)))
    //                {
    //                    notifyErrorLog("超时未收到数据，S+Ka遥测+Ka数传快速校相失败");
    //                    break;
    //                }
    //            }
    //        }
    //        //如果时S遥测
    //        if (currentFreqBand == AcuFreqBandStatusS && m_finishCalibPhaseMap.contains(1))
    //        {
    //            notifyInfoLog("S频段快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusKaYC && m_finishCalibPhaseMap.contains(2))
    //        {
    //            notifyInfoLog("Ka遥测快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusKaDT && m_finishCalibPhaseMap.contains(3))
    //        {
    //            notifyInfoLog("Ka数传快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusS_KaYC && m_finishCalibPhaseMap.contains(1) && m_finishCalibPhaseMap.contains(3))
    //        {
    //            notifyInfoLog("S+Ka遥测快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusS_KaDT && m_finishCalibPhaseMap.contains(1) && m_finishCalibPhaseMap.contains(2))
    //        {
    //            notifyInfoLog("S+Ka数传快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusS_KaYC_KaDT && m_finishCalibPhaseMap.contains(2) && m_finishCalibPhaseMap.contains(3))
    //        {
    //            notifyInfoLog("Ka遥测+Ka数传快速校相完成");
    //            break;
    //        }

    //        if (currentFreqBand == AcuFreqBandStatusS_KaYC_KaDT && m_finishCalibPhaseMap.contains(1) && m_finishCalibPhaseMap.contains(2) &&
    //            m_finishCalibPhaseMap.contains(3))
    //        {
    //            notifyInfoLog("S+Ka遥测+Ka数传快速校相完成");
    //            break;
    //        }

    //        if (i == 120 && !recvIndex)
    //        {
    //            //校相失败 ACU没有应答 60s没有应答直接提示校相失败
    //            notifyErrorLog(QString("校相失败，ACU没有进行快速校相"));
    //            break;
    //        }
    //}
    //    if(!isSuccess){
    //        notifyInfoLog(QString("校相失败，控制ACU进入程序引导"));
    //        // 控制ACU进入程序引导
    //        cmd.getTaskPlanCmd(array, QString("Step_AC_5"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID,
    //        m_runningCmdID,{},deviceACUMaster); waitExecSuccess(array);
    //    }

    //    //校相完成后给ACU下发任务开始命令
    //    QMap<QString, QVariant> acuStartTaskMap;
    //    acuStartTaskMap["TaskIdent"] = acuTaskIndent;
    //    m_singleCommandHelper.packSingleCommand("Step_ACU_TASK_START", packCommand, deviceACUMaster, acuStartTaskMap);
    //    waitExecSuccess(packCommand);

    //    VoiceAlarmPublish::publish("跟踪开始");

    //    notifyInfoLog("校相处理流程完成");
    //    QThread::sleep(1);
    //    notify("CaptureTrackIng");
    // notify("GoUp");
}

void Calibration::calibrationError(const QString& logStr)
{
    notifyErrorLog(logStr);
    VoiceAlarmPublish::publish("校相错误");
}

void Calibration::trackTaskStart()
{
    //校相完成后给ACU下发任务开始命令
    QMap<QString, QVariant> acuStartTaskMap;
    acuStartTaskMap["TaskIdent"] = m_taskTimeList.masterTaskBz;
    m_singleCommandHelper.packSingleCommand("Step_ACU_TASK_START", m_packCommand, m_taskTimeList.onlineACU, acuStartTaskMap);
    waitExecSuccess(m_packCommand);

    VoiceAlarmPublish::publish("校相结束");
    VoiceAlarmPublish::publish("跟踪开始");
    QThread::sleep(1);
    notify("CaptureTrackIng");
}

void Calibration::programTaskStart()
{
    // 获取之前的相位数据
    auto workMode = TaskRunCommonHelper::trackingToSystemWorkMode(m_taskTimeList.trackingMode);
    QVariant downFreq;
    MacroCommon::getDownFrequency(m_taskTimeList.masterParamMacroData, workMode, 1, downFreq);
    auto pcInfo = TaskGlobalInfoSingleton::getInstance().getCalibResultInfo(workMode, m_taskTimeList.masterTaskCode, downFreq.toDouble());

    DeviceID deviceID;
    if (m_taskTimeList.manualMessage.configMacroData.getGZJDDeviceID(workMode, deviceID))
    {
        // 下发给跟踪基带
        if (workMode == SKT)
        {
            QVariantMap replaceMap;
            replaceMap["AzimPhase1"] = pcInfo.azge;
            replaceMap["PitchPhase1"] = pcInfo.elge;
            replaceMap["AzimSlope"] = pcInfo.azpr;
            replaceMap["PitchSlope"] = pcInfo.elpr;
            m_singleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_KT", m_packCommand, deviceID, replaceMap);
            waitExecSuccess(m_packCommand);
        }
        else if (SystemWorkModeHelper::isDataTransmissionWorkMode(workMode))
        {
            QVariantMap replaceMap;
            replaceMap["AzimPhase"] = pcInfo.azge;
            replaceMap["PitchPhase"] = pcInfo.elge;
            replaceMap["AzimSlope"] = pcInfo.azpr;
            replaceMap["PitchSlope"] = pcInfo.elpr;
            m_singleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_YC", m_packCommand, deviceID, replaceMap);
            waitExecSuccess(m_packCommand);
        }
        else if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            QVariantMap replaceMap;
            replaceMap["AzimPhase"] = pcInfo.azge;
            replaceMap["PitchPhase"] = pcInfo.elge;
            replaceMap["AzimSlope"] = pcInfo.azpr;
            replaceMap["PitchSlope"] = pcInfo.elpr;
            m_singleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_SC", m_packCommand, deviceID, replaceMap);
            waitExecSuccess(m_packCommand);
        }
    }

    //校相完成后给ACU下发任务开始命令
    QMap<QString, QVariant> acuStartTaskMap;
    acuStartTaskMap["TaskIdent"] = m_taskTimeList.masterTaskBz;
    m_singleCommandHelper.packSingleCommand("Step_ACU_TASK_START", m_packCommand, m_taskTimeList.onlineACU, acuStartTaskMap);
    waitExecSuccess(m_packCommand);

    VoiceAlarmPublish::publish("跟踪开始");
    QThread::sleep(1);
    notify("CaptureTrackIng");
}

void Calibration::addXuanXiang(QVariantMap& acuCalibrationParamMap)
{
    auto getTrackPolar = [&](SystemWorkMode workMode) {
        SystemOrientation systemOrientation = SystemOrientation::Unkonwn;
        m_taskTimeList.manualMessage.configMacroData.getTrackPolar(workMode, systemOrientation);
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
    if (m_taskTimeList.trackingMode == _4426_SK2 ||              //
        m_taskTimeList.trackingMode == _4426_SK2_KaCk ||         //
        m_taskTimeList.trackingMode == _4426_SK2_KaDSDT ||       //
        m_taskTimeList.trackingMode == _4426_SK2_KaGSDT ||       //
        m_taskTimeList.trackingMode == _4426_SK2_KaCk_KaDSDT ||  //
        m_taskTimeList.trackingMode == _4426_SK2_KaCK_KaGSDT)
    {
        sXuanxiang = getTrackPolar(Skuo2);
    }
    else if (m_taskTimeList.trackingMode == _4426_SKT)
    {
        sXuanxiang = getTrackPolar(SKT);
    }

    // 默认是低速 所以只检查高速
    if (m_taskTimeList.trackingMode == _4426_KaGSDT ||            //
        m_taskTimeList.trackingMode == _4426_STTC_KaGSDT ||       //
        m_taskTimeList.trackingMode == _4426_STTC_KaCK_KaGSDT ||  //
        m_taskTimeList.trackingMode == _4426_SK2_KaGSDT ||        //
        m_taskTimeList.trackingMode == _4426_SK2_KaCK_KaGSDT)
    {
        kascXuanxiang = getTrackPolar(KaGS);
    }
    acuCalibrationParamMap["SEmissPolar"] = sXuanxiang;        // S接收极化 1：左旋 2：右旋
    acuCalibrationParamMap["KaSCEmissPolar"] = kascXuanxiang;  // Ka数传接收极化
    acuCalibrationParamMap["KaYCEmissPolar"] = kaycXuanxiang;  // Ka接收极化 1：左旋 2：右旋
}
