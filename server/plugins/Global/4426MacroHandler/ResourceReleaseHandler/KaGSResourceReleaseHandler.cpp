#include "KaGSResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"

KaGSResourceReleaseHandler::KaGSResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}

bool KaGSResourceReleaseHandler::handle()
{
    /* 高速是 */
    clearQueue();

    releaseItem(DeviceID(4, 1, 1));
    releaseItem(DeviceID(4, 1, 2));

    execQueue();
    return true;
}

void KaGSResourceReleaseHandler::releaseItem(const DeviceID& deviceID)
{
    // 设置存盘停止
    // 20211120 存盘的I/Q参数是无效的 但是也要设置为1或者2
    //通道一
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I1", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q1", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    //通道二
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I2", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q2", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    //设置送数停止
    //通道一
    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Stop_1", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    //通道二
    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Stop_2", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
}
SystemWorkMode KaGSResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::KaGS; }
