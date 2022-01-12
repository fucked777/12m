#include "DSResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"

DSResourceReleaseHandler::DSResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}

bool DSResourceReleaseHandler::handle()
{
    clearQueue();

    releaseItem(DeviceID(4, 4, 1));
    releaseItem(DeviceID(4, 4, 2));

    execQueue();

    return true;
}
void DSResourceReleaseHandler::releaseItem(const DeviceID& deviceID)
{
    // 设置存盘停止
    //通道一
    mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    //设置送数停止
    mSingleCommandHelper.packSingleCommand("StepDSJDSS_Stop_Ka", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    // 设置存盘停止
    //通道一
    mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    //设置送数停止
    mSingleCommandHelper.packSingleCommand("StepDSJDSS_Stop_X", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
}
SystemWorkMode DSResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::KaDS; }
