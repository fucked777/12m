#include "SPMNYYXBHLinkHandler.h"

#include "GlobalData.h"
#include "SingleCommandHelper.h"
#include "SystemWorkMode.h"

SPMNYYXBHLinkHandler::SPMNYYXBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool SPMNYYXBHLinkHandler::handle()
{
    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        // 获取主用测控基带ID
        DeviceID ckDeviceID;
        if (!getCKJDDeviceID(ckDeviceID))
        {
            return false;
        }

        // 根据的配置，切换测控基带主备
        switchCKJDMasterSlave();

        // 进行切换基带的工作模式
        if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
        {
            emit signalErrorMsg(QString("%1 切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
            return false;
        }

        // 从配置宏中获取测控前端
        DeviceID ckqdDeviceID;
        if (!getCKQDDeviceID(ckqdDeviceID))
        {
            return false;
        }

        // 只用下发一次光口的配置
        setLightNetConfigCKJD_CKQD(ckDeviceID, ckqdDeviceID);
    }
    // X高速
    else if (mLinkLine.workMode == XGS)
    {
        // 切换高速基带主备
        if (!switchGSJDMasterSlave())
        {
            return false;
        }

        //        // 配置宏获取X下变频器设备ID
        //        DeviceID xxbpqDeviceID;
        //        if (!getXXBPQDeviceID(xxbpqDeviceID))
        //        {
        //            return false;
        //        }

        //        // X模拟源环路是需要将高速基带工作方式设置为中频闭环
        //        singleCmdHelper.packSingleCommand("StepGS_WORK_SWITCH_ZPBH", packCommand, gsjdDeviceID);
        //        waitExecSuccess(packCommand);

        //        // 切换信号给高速基带 根据选择的高数基带和X下变频器组合发送不同的命令
        //        QString singleCmdId = QString("StepZPKGJZ_X_%1_%2").arg(gsjdDeviceID.extenID).arg(xxbpqDeviceID.extenID);
        //        singleCmdHelper.packSingleCommand(singleCmdId, packCommand);
        //        waitExecSuccess(packCommand);
    }
    // Ka高速
    else if (mLinkLine.workMode == KaGS)
    {
        //        // 获取高速基带ID
        //        DeviceID gsjdDeviceID;
        //        if (!getMasterGSJDDeviceID(gsjdDeviceID))
        //        {
        //            return false;
        //        }

        //        // Ka模拟源环路是需要将工作方式设置为中频闭环
        //        singleCmdHelper.packSingleCommand("StepGS_WORK_SWITCH_ZPBH", packCommand, gsjdDeviceID);
        //        waitExecSuccess(packCommand);
    }

    // 执行链路命令
    execLinkCommand();

    return true;
}

LinkType SPMNYYXBHLinkHandler::getLinkType() { return LinkType::SPMNYYXBH; }
