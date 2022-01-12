#include "ZPBHLinkHandler.h"

#include "GlobalData.h"
#include "SingleCommandHelper.h"

ZPBHLinkHandler::ZPBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool ZPBHLinkHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 测控模式直接返回
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return false;
    }

    // 执行链路命令
    execLinkCommand();

    // 获取高速基带ID
    QList<DeviceID> gsjdDeviceIDList;
    getGSJDDeviceIDS(gsjdDeviceIDList);

    // 设置高速基带为中频闭环
    for (auto deviceID : gsjdDeviceIDList)
    {
        singleCmdHelper.packSingleCommand("StepGS_WORK_SWITCH_ZPBH", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }

    return true;
}

LinkType ZPBHLinkHandler::getLinkType() { return LinkType::ZPBH; }
