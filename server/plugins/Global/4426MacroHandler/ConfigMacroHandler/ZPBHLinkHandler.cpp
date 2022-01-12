#include "ZPBHLinkHandler.h"

#include "GlobalData.h"

ZPBHLinkHandler::ZPBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool ZPBHLinkHandler::handle()
{
    // 测控模式直接返回
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return false;
    }

    // 高速模式
    if (mLinkLine.workMode == KaGS)
    {
        // 执行链路命令
        execLinkCommand();
        ExitCheck(false);

        auto result = getGSJDDeviceID();
        auto msg = result.msg();
        if (!msg.isEmpty())
        {
            emit signalErrorMsg(msg);
        }
        if (!result)
        {
            return false;
        }
        auto gsDeviceInfo = result.value();
        // 切换高速基带主备
        if (!switchGSJDMasterSlave())
        {
            return false;
        }

        // 高速基带设置为数字化闭环
        mSingleCommandHelper.packSingleCommand("StepGS_ZPBH", mPackCommand, gsDeviceInfo.master);
        waitExecSuccess(mPackCommand);
    }
    // 低速模式
    else if (mLinkLine.workMode == XDS || mLinkLine.workMode == KaDS)
    {
        // 执行链路命令
        execLinkCommand();
        ExitCheck(false);

        // 测控前端
        auto ckqdLinkResult = ckqdLink(CKQDMode::SZHBH);
        // 低速基带
        auto dsjdLinkResult = dsjdLink();

        if (ckqdLinkResult && dsjdLinkResult)
        {
            // 设置低速基带和测控前端光口网络
            setLightNetConfigDSJD_CKQD(m_dsjdDeviceID, m_ckqdDeviceInfo.master);
        }
    }

    return true;
}

LinkType ZPBHLinkHandler::getLinkType() { return LinkType::ZPBH; }
bool ZPBHLinkHandler::dsjdLink()
{
    /* 基带 */
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
    auto info = result.value();
    if (!info.master.isValid())
    {
        return false;
    }

    m_dsjdDeviceID = info.master;

    // 切换低速基带主备
    if (!switchDSJDMasterSlave())
    {
        return false;
    }

    // 低速基带设置为数字化闭环
    mSingleCommandHelper.packSingleCommand(mLinkLine.workMode == XDS ? "StepXDS_ZPBH" : "StepKaDS_ZPBH", mPackCommand, m_dsjdDeviceID);
    waitExecSuccess(mPackCommand);
    return true;
}
