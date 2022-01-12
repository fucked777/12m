#include "SZHBHLinkHandler.h"

#include "GlobalData.h"

SZHBHLinkHandler::SZHBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool SZHBHLinkHandler::handle()
{
    // 数传模式直接返回
    if (SystemWorkModeHelper::isDataTransmissionWorkMode(mLinkLine.workMode))
    {
        return false;
    }

    // 使用主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    // auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    // auto masterTargetWorkMode = masterTargetInfo.workMode;

    // 获取输出电平
    QVariant outLevel;
    MacroCommon::getOutputLevel(masterTargetTaskCode, mLinkLine.workMode, outLevel);
    QMap<QString, QVariant> outputLevelMap;
    outputLevelMap["OutputLev"] = outLevel;

    // 测控基带
    bool ckjdLinkResult = ckjdLink();
    ExitCheck(false);

    // 测控前端
    auto ckqdLinkResult = ckqdLink(CKQDMode::SZHBH);
    ExitCheck(false);

    if (!ckjdLinkResult)
    {
        return false;
    }
    // 只用下发一次光口的配置
    if (ckqdLinkResult)
    {
        setLightNetConfigCKJD_CKQD(m_ckjdInfo, m_ckqdDeviceInfo.master);
    }

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        // 测控基带工作方式设置成数字化闭环
        mSingleCommandHelper.packSingleCommand("StepSTTC_SZHBH", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepSTTC_SZHBH", mPackCommand, m_ckjdInfo.afterSlave);
        waitExecSuccess(mPackCommand, 0);

        // 上行载波输出
        mSingleCommandHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);

        // 捕获方式设置为自动
        mSingleCommandHelper.packSingleCommand("StepSTTC_BH_AUTO", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
        QThread::sleep(2);
        // 双捕开始
        mSingleCommandHelper.packSingleCommand("StepSTTC_SB_START", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        // 测控基带工作方式设置成数字化闭环
        mSingleCommandHelper.packSingleCommand("StepSkuo2_SZHBH", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepSkuo2_SZHBH", mPackCommand, m_ckjdInfo.afterSlave);
        waitExecSuccess(mPackCommand, 0);

        // 上行载波输出
        mSingleCommandHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
    }
    // Ka扩二
    else if (mLinkLine.workMode == KaKuo2)
    {
        // 测控基带工作方式设置成数字化闭环
        mSingleCommandHelper.packSingleCommand("StepKakuo2_SZHBH", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepKakuo2_SZHBH", mPackCommand, m_ckjdInfo.afterSlave);
        waitExecSuccess(mPackCommand, 0);

        // 上行载波输出
        mSingleCommandHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
    }
    // 扩跳
    else if (mLinkLine.workMode == SKT)
    {
        // 测控基带工作方式设置成数字化闭环
        mSingleCommandHelper.packSingleCommand("StepSKT_SZHBH", mPackCommand);
        waitExecSuccess(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepSKT_SZHBH", mPackCommand, m_ckjdInfo.afterSlave);
        waitExecSuccess(mPackCommand, 0);

        // 测控基带基带上行载波输出
        mSingleCommandHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", mPackCommand, m_ckjdInfo.afterMaster, outputLevelMap);
        waitExecSuccess(mPackCommand);
    }

    return true;
}

LinkType SZHBHLinkHandler::getLinkType() { return LinkType::SZHBH; }
bool SZHBHLinkHandler::ckjdLink()
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
