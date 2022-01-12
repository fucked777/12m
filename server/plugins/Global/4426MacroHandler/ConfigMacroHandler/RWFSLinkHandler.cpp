#include "RWFSLinkHandler.h"

#include "GlobalData.h"
#include "RedisHelper.h"
#include <QDebug>

RWFSLinkHandler::RWFSLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool RWFSLinkHandler::handle()
{
    // 如果是测控模式
    auto result = false;
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        result = handleCK();
    }
    // 如果是高速数传模式
    else if (SystemWorkModeHelper::isGsDataTransmissionWorkMode(mLinkLine.workMode))
    {
        result = handleGS();
    }
    //如果是低速数传模式
    else if (SystemWorkModeHelper::isDsDataTransmissionWorkMode(mLinkLine.workMode))
    {
        result = handleDS();
    }

    return result;
}

LinkType RWFSLinkHandler::getLinkType() { return LinkType::RWFS; }
#if 1

bool RWFSLinkHandler::handleCK()
{
    m_workModeSet.clear();
    // 判断是否是双测控
    for (auto iter = mManualMsg.linkLineMap.begin(); iter != mManualMsg.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            m_workModeSet << workMode;
        }
    }

    /* 提前判断一下 双模式是S扩2+Ka扩2对于测控基带只下一次宏就行
     * 双主机也只下一次宏
     * 跳过第二次下宏
     */
    if ((m_workModeSet.size() >= 2 && (mLinkLine.workMode == *(m_workModeSet.begin()))) || m_workModeSet.size() == 1)
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
            setLightNetConfigCKJD_CKQD(m_ckjdInfo, m_ckqdDeviceInfo.master, isDoubleMode());
        }
        ExitCheck(false);
    }

    // 执行链路命令
    execLinkCommand();

    return true;
}

#else

bool RWFSLinkHandler::handleCK()
{
    m_workModeSet.clear();
    // 判断是否是双测控
    for (auto iter = mManualMsg.linkLineMap.begin(); iter != mManualMsg.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            m_workModeSet << workMode;
        }
    }

    /* 提前判断一下 双模式是S扩2+Ka扩2对于测控基带只下一次宏就行
     * 双主机也只下一次宏
     * 跳过第二次下宏
     */
    if ((m_workModeSet.size() >= 2 && (mLinkLine.workMode == *(m_workModeSet.begin()))) || m_workModeSet.size() == 1)
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
            setLightNetConfigCKJD_CKQD(m_ckjdInfo, m_ckqdDeviceInfo.master, isDoubleMode());
        }
        ExitCheck(false);

        // 执行链路命令
        execLinkCommand(m_workModeSet);
    }

    return true;
}

#endif
bool RWFSLinkHandler::ckjdLink()
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
    return (m_workModeSet.size() == 1 ? ckjdLinkSingle() : ckjdLinkDouble());
}
bool RWFSLinkHandler::ckjdLinkSingle()
{
    // 切换测控基带的工作模式
    auto switchRes = switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode);
    if (CKJDSwitchStatus::Failed == switchRes)
    {
        return false;
    }

    clearQueue();
    if ((switchRes == CKJDSwitchStatus::All || switchRes == CKJDSwitchStatus::Master) && m_ckjdInfo.afterMaster.isValid())
    {
        // 设置测控基带为任务方式
        mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, m_ckjdInfo.afterMaster, mLinkLine.workMode);
        appendExecQueue(mPackCommand);
    }

    if ((switchRes == CKJDSwitchStatus::All || switchRes == CKJDSwitchStatus::Slave) && m_ckjdInfo.afterSlave.isValid())
    {
        // 设置测控基带为任务方式
        mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, m_ckjdInfo.afterSlave, mLinkLine.workMode);
        appendExecQueue(mPackCommand);
    }
    execQueue();
    // 根据的配置，切换测控基带主备
    switchCKJDMasterSlave(m_ckjdInfo);

    return true;
}
bool RWFSLinkHandler::ckjdLinkDouble()
{
    // TODO 818
    // 切换测控基带的工作模式
    auto switchRes = switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode);
    if (CKJDSwitchStatus::Failed == switchRes)
    {
        return false;
    }

    // 根据的配置，切换测控基带主备
    switchCKJDMasterSlave(m_ckjdInfo);

    clearQueue();
    if ((switchRes == CKJDSwitchStatus::All || switchRes == CKJDSwitchStatus::Master) && m_ckjdInfo.afterMaster.isValid())
    {
        // 设置测控基带为任务方式
        mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, m_ckjdInfo.afterMaster, mLinkLine.workMode);
        appendExecQueue(mPackCommand);

        if (m_ckjdInfo.afterDoubleMode)
        {
            // 这里 双模式是skuo2和kakuo他们的模式切换命令ID是一样的 用同一个模式都行
            auto tempDevice = getEqModeCKDeviceID(m_ckjdInfo.afterMaster);
            // 设置测控基带为任务方式
            mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, tempDevice, mLinkLine.workMode);
            appendExecQueue(mPackCommand);
        }
    }
    if ((switchRes == CKJDSwitchStatus::All || switchRes == CKJDSwitchStatus::Slave) && m_ckjdInfo.afterSlave.isValid())
    {
        // 设置测控基带为任务方式
        mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, m_ckjdInfo.afterSlave, mLinkLine.workMode);
        appendExecQueue(mPackCommand);

        if (m_ckjdInfo.afterDoubleMode)
        {
            // 这里 双模式是skuo2和kakuo他们的模式切换命令ID是一样的 用同一个模式都行
            auto tempDevice = getEqModeCKDeviceID(m_ckjdInfo.afterSlave);
            // 设置测控基带为任务方式
            mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, tempDevice, mLinkLine.workMode);
            appendExecQueue(mPackCommand);
        }
    }
    execQueue();
    if (m_ckjdInfo.afterDoubleMode)
    {
        return true;
    }

    /* 另一台切换 */
    if (!m_ckjdInfo.beforeSlave.isValid())
    {
        emit signalErrorMsg(QString("%1当前设备数据获取失败,设备可能不在线").arg(GlobalData::getExtensionName(m_ckjdInfo.beforeMaster)));
        return false;
    }
    if (!switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode, false))
    {
        emit signalErrorMsg(QString("%1切换模式失败，预设工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(m_ckjdInfo.beforeSlave)));
        return false;
    }

    // 设置为主机
    switchJDMasterSlave(m_ckjdInfo.afterSlave, MasterSlave::Master);

    // 设置测控基带为任务方式
    mSingleCommandHelper.packSingleCommand("StepCKJD_RWFS", mPackCommand, m_ckjdInfo.afterSlave, mLinkLine.workMode);
    waitExecSuccess(mPackCommand);

    return true;
}
bool RWFSLinkHandler::dsjdLink()
{
    auto result = getDSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }
    m_dsjdInfo = result.value();
    // 切换低速基带主备
    switchDSJDMasterSlave(m_dsjdInfo);

    return true;
}
int RWFSLinkHandler::dsModeNum() const
{
    auto dsNum = static_cast<int>(mManualMsg.linkLineMap.contains(XDS));
    dsNum += static_cast<int>(mManualMsg.linkLineMap.contains(KaDS));
    return dsNum;
}
bool RWFSLinkHandler::handleDS()
{
    auto dsNum = dsModeNum();
    /* 只执行一次前端和光口网络配置 */
    if ((dsNum >= 2 && mLinkLine.workMode == KaDS) || dsNum == 1)
    {
        // 低速基带
        auto dsjdLinkResult = dsjdLink();
        ExitCheck(false);

        // 测控前端
        auto ckqdLinkResult = ckqdLink(CKQDMode::RW);
        ExitCheck(false);

        if (dsjdLinkResult && ckqdLinkResult)
        {
            // 设置低速基带和测控前端光口网络
            setLightNetConfigDSJD_CKQD(m_dsjdInfo.master, m_ckqdDeviceInfo.master);
        }

        clearQueue();
        // 设置低速基带为任务方式
        if (m_dsjdInfo.master.isValid())
        {
            mSingleCommandHelper.packSingleCommand("StepXDS_RWFS", mPackCommand, m_dsjdInfo.master);
            appendExecQueue(mPackCommand);
        }

        if (m_dsjdInfo.slave.isValid())
        {
            mSingleCommandHelper.packSingleCommand("StepXDS_RWFS", mPackCommand, m_dsjdInfo.slave);
            appendExecQueue(mPackCommand);
        }
        execQueue();
    }
    // 执行链路命令
    execLinkCommand();

    return true;
}
bool RWFSLinkHandler::handleGS()
{
    // 切换高速基带主备
    switchGSJDMasterSlave();

    // 获取全部高速基带ID
    QList<DeviceID> gsjdDeviceIDList;
    getGSJDDeviceIDS(gsjdDeviceIDList);

    clearQueue();
    // 设置高速基带为任务方式
    for (auto& gsjdDeviceID : gsjdDeviceIDList)
    {
        mSingleCommandHelper.packSingleCommand("StepGS_WORK_SWITCH_RWFS", mPackCommand, gsjdDeviceID);
        appendExecQueue(mPackCommand);
    }
    execQueue();

    // 执行链路命令
    execLinkCommand();
    return true;
}
