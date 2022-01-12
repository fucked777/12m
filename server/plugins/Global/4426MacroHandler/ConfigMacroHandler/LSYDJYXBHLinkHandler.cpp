#include "LSYDJYXBHLinkHandler.h"

#include "GlobalData.h"
#include "SystemWorkMode.h"

LSYDJYXBHLinkHandler::LSYDJYXBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool LSYDJYXBHLinkHandler::handle()
{
    // 不是测控模式直接跳过
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return false;
    }

    // 使用主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    // auto masterTargetWorkMode = masterTargetInfo.workMode;

    // 获取上行和下行的频率
    QVariant upFrequency;
    QVariant downFrequency;
    if (!MacroCommon::getFrequency(masterTargetTaskCode, mLinkLine.workMode, masterTargetPointFreqNo, upFrequency, downFrequency))
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);
        auto msg = QString("获取任务代号为%1 工作模式为%2 点频为%3的参数宏上下行频率错误")  //
                       .arg(masterTargetTaskCode, workModeDesc)                             //
                       .arg(masterTargetPointFreqNo);
        emit signalErrorMsg(msg);
        return false;
    }

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

    ExitCheck(false);
    execLinkCommand();

    if (mLinkLine.workMode == STTC || mLinkLine.workMode == Skuo2 || mLinkLine.workMode == SKT)
    {
        // S频段信道接收、发射频率
        QMap<QString, QVariant> frequencyMap;
        frequencyMap["TranJointTran"] = downFrequency;
        frequencyMap["ReceFreqResp"] = upFrequency;

        mSingleCommandHelper.packSingleCommand("StepLSYDJXD_XD_S", mPackCommand, frequencyMap);
        waitExecSuccess(mPackCommand);
    }
    else if (mLinkLine.workMode == KaKuo2)
    {
        // Ka频段信道接收、发射频率
        QMap<QString, QVariant> frequencyMap;
        frequencyMap["TranJointTran"] = downFrequency;
        frequencyMap["ReceFreqResp"] = upFrequency;

        mSingleCommandHelper.packSingleCommand("StepLSYDJXD_XD_Ka", mPackCommand, frequencyMap);
        waitExecSuccess(mPackCommand);
    }

    // 非扩跳模式时，切换联试应答机模式
    if (mLinkLine.workMode == STTC)
    {
        mSingleCommandHelper.packSingleCommand("StepLSYDJ_WorkModeSwitch_STTC", mPackCommand);
        waitExecSuccess(mPackCommand);
    }
    else if (mLinkLine.workMode == KaKuo2 || mLinkLine.workMode == Skuo2)
    {
        mSingleCommandHelper.packSingleCommand("StepLSYDJ_WorkModeSwitch_KP", mPackCommand);
        waitExecSuccess(mPackCommand);
    }
    return true;
}

LinkType LSYDJYXBHLinkHandler::getLinkType() { return LinkType::LSYDJYXBH; }
bool LSYDJYXBHLinkHandler::ckjdLink()
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
