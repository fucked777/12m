#include "RWFSLinkHandler.h"

#include "GlobalData.h"
#include "RedisHelper.h"
#include "SingleCommandHelper.h"

RWFSLinkHandler::RWFSLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool RWFSLinkHandler::handle()
{
    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }

    // 如果是测控模式
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return handleCKRWFS();
    }
    // 是Ka/X数传模式
    else
    {
        return handleDTRWFS();
    }
}

LinkType RWFSLinkHandler::getLinkType() { return LinkType::RWFS; }

bool RWFSLinkHandler::handleCKRWFS()
{
    // 执行链路命令
    execLinkCommand();

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 获取测控前端和测控基带的DeviceID ，用于配置光口网络的信号
    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }
    // 切换测控基带的工作模式
    if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("%1 切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
        return false;
    }

    // 设置测控基带为任务方式
    singleCmdHelper.packSingleCommand("StepCKJD_RWFS", packCommand, ckDeviceID, mLinkLine.workMode);
    waitExecSuccess(packCommand);

    // 获取测控前端
    DeviceID ckqdDeviceID;
    if (!getCKQDDeviceID(ckqdDeviceID))
    {
        return false;
    }

    // 切换前端的工作方式为任务方式
    singleCmdHelper.packSingleCommand("StepQD_WORK_RWFS", packCommand, ckqdDeviceID);
    waitExecSuccess(packCommand);

    // 只用下发一次光口的配置
    setLightNetConfigCKJD_CKQD(ckDeviceID, ckqdDeviceID);

    // 切换跟踪基带的工作方式为任务方式
    //        singleCmdHelper.packSingleCommand("StepGZGS_RWFS", packCommand);
    //        waitExecSuccess(packCommand);

    return true;
}

bool RWFSLinkHandler::handleDTRWFS()
{
    // 执行链路命令
    execLinkCommand();

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 切换高速基带主备
    if (!switchGSJDMasterSlave())
    {
        return false;
    }

    // 获取全部高速基带ID
    QList<DeviceID> gsjdDeviceIDList;
    getGSJDDeviceIDS(gsjdDeviceIDList);

    // 设置高速基带为任务方式
    for (auto gsjdDeviceID : gsjdDeviceIDList)
    {
        singleCmdHelper.packSingleCommand("StepGS_WORK_SWITCH_RWFS", packCommand, gsjdDeviceID);
        waitExecSuccess(packCommand);
    }

    return true;
}
