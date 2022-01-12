#include "PKXLBPQBHLinkHandler.h"

#include "GlobalData.h"
#include "PKXLBPQBHLinkHandler.h"
#include "SystemWorkMode.h"

PKXLBPQBHLinkHandler::PKXLBPQBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool PKXLBPQBHLinkHandler::handle()
{
    // 不是测控模式直接跳过
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return false;
    }

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

LinkType PKXLBPQBHLinkHandler::getLinkType() { return LinkType::PKXLBPQBH; }

bool PKXLBPQBHLinkHandler::ckjdLink()
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
bool PKXLBPQBHLinkHandler::ckjdLinkSingle()
{
    // 切换测控基带的工作模式
    auto switchRes = switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode);
    if (CKJDSwitchStatus::Failed == switchRes)
    {
        return false;
    }

    // 根据的配置，切换测控基带主备
    switchCKJDMasterSlave(m_ckjdInfo);

    return true;
}
bool PKXLBPQBHLinkHandler::ckjdLinkDouble()
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

    return true;
}
