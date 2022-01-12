#include "SPMNYYXBHLinkHandler.h"

#include "GlobalData.h"
#include "SystemWorkMode.h"

SPMNYYXBHLinkHandler::SPMNYYXBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool SPMNYYXBHLinkHandler::handle()
{
    // 使用主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    // auto masterTargetWorkMode = masterTargetInfo.workMode;

    // 获取上行和下行的频率
    QVariant downFrequency;
    if (!MacroCommon::getDownFrequency(masterTargetTaskCode, mLinkLine.workMode, masterTargetPointFreqNo, downFrequency))
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);
        auto msg = QString("获取任务代号为%1 工作模式为%2 点频为%3的参数宏下行频率错误")  //
                       .arg(masterTargetTaskCode, workModeDesc)                           //
                       .arg(masterTargetPointFreqNo);
        emit signalErrorMsg(msg);
        return false;
    }

    // 如果是测控模式
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        // 测控基带
        bool ckjdLinkResult = ckjdLink();
        ExitCheck(false);
        // 测控前端
        auto ckqdLinkResult = ckqdLink(CKQDMode::RW);
        ExitCheck(false);

        // 只用下发一次光口的配置
        if (ckjdLinkResult && ckqdLinkResult)
        {
            setLightNetConfigCKJD_CKQD(m_ckjdInfo, m_ckqdDeviceInfo.master);
        }
    }
    // 如果低速模式
    else if (mLinkLine.workMode == XDS)
    {
        // 切换低速基带主备
        switchDSJDMasterSlave();

        // 测试高频箱X的数据 X频段频率设置
        QMap<QString, QVariant> otherReplaceMap;
        otherReplaceMap["XTestOutputFreq"] = downFrequency;
        mSingleCommandHelper.packSingleCommand("StepCSGPX_X_OUTPUT", mPackCommand, otherReplaceMap);
        waitExecSuccess(mPackCommand);

        // 前端测试开关网络选择X模拟信号出
        mSingleCommandHelper.packSingleCommand("StepXLS_QDMNY", mPackCommand);
        waitExecSuccess(mPackCommand);

        // 设置X频段高频箱和路变频器频率
        QMap<QString, QVariant> XHLBPQParamMap;
        XHLBPQParamMap["SBandDownFreq"] = downFrequency;
        for (int number = 1; number <= 2; ++number)
        {
            mSingleCommandHelper.packSingleCommand(QString("Step_XPDGPX_HLBPQ_%1").arg(number), mPackCommand, XHLBPQParamMap);
            waitExecSuccess(mPackCommand);
        }
    }
    else if (mLinkLine.workMode == KaDS)
    {
        // 切换低速基带主备
        switchDSJDMasterSlave();

        // 测试高频箱选择Ka输出
        QMap<QString, QVariant> otherReplaceMap;
        otherReplaceMap["KaTestOutputFreq"] = downFrequency;
        mSingleCommandHelper.packSingleCommand("StepCSGPX_KA_OUTPUT", mPackCommand, otherReplaceMap);
        waitExecSuccess(mPackCommand);

        // 前端测试开关网络选择ka低速/测控模拟信号出
        mSingleCommandHelper.packSingleCommand("StepKALS_QDMNY", mPackCommand);
        waitExecSuccess(mPackCommand);

        // 设置Ka低速速传及跟踪高频箱 Ka/S下变频器频率
        QMap<QString, QVariant> XHLBPQParamMap;
        XHLBPQParamMap["SBandDownFreq"] = downFrequency;
        for (int number = 1; number <= 6; ++number)
        {
            mSingleCommandHelper.packSingleCommand(QString("Step_KaDSSCJGZGPX_HLBPQ_%1").arg(number), mPackCommand, XHLBPQParamMap);
            waitExecSuccess(mPackCommand);
        }
    }
    else if (mLinkLine.workMode == KaGS)
    {
        // 切换高速基带主备
        switchGSJDMasterSlave();

        // 测试高频箱Ka高速的数据
        QMap<QString, QVariant> otherReplaceMap;
        otherReplaceMap["KaAnalogCtrls"] = downFrequency;
        mSingleCommandHelper.packSingleCommand("StepCSGPX_KAHS_OUTPUT", mPackCommand, otherReplaceMap);
        waitExecSuccess(mPackCommand);

        // 前端测试开关网络选择Ka高速模拟信号出
        mSingleCommandHelper.packSingleCommand("StepKAHS_QDMNY", mPackCommand);
        waitExecSuccess(mPackCommand);

        // 设置ka高速数传高频箱下变频器频率
        QMap<QString, QVariant> kaGSSCGPXParamMap;
        kaGSSCGPXParamMap["SBandDownFreq"] = downFrequency;
        for (int number = 1; number <= 4; ++number)
        {
            mSingleCommandHelper.packSingleCommand(QString("Step_KaGSSCGPX_XBPQ_%1").arg(number), mPackCommand, kaGSSCGPXParamMap);
            waitExecSuccess(mPackCommand);
        }
    }

    ExitCheck(false);
    // 执行链路命令
    execLinkCommand();

    return true;
}

LinkType SPMNYYXBHLinkHandler::getLinkType() { return LinkType::SPMNYYXBH; }
bool SPMNYYXBHLinkHandler::ckjdLink()
{
    // 获取测控基带ID
    auto result = getCKJDControlInfo(mLinkLine.workMode);
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(result.msg());
    }
    if (!result)
    {
        return false;
    }
    m_ckjdInfo = result.value();
    // 进行切换基带的工作模式
    if (CKJDSwitchStatus::Failed == switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("%1 切换模式失败，预设工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(m_ckjdInfo.beforeMaster)));
        return false;
    }
    // 根据的配置，切换测控基带主备
    switchCKJDMasterSlave(m_ckjdInfo);
    return true;
}
