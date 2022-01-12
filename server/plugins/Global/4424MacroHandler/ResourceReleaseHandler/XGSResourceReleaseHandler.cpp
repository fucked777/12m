#include "XGSResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "SingleCommandHelper.h"

XGSResourceReleaseHandler::XGSResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}

bool XGSResourceReleaseHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 获取Ka高速基带设备ID 主用
    DeviceID masterGSJDDeviceID;
    if (!getGSJDDeviceID(masterGSJDDeviceID, 1))
    {
        return false;
    }

    // 设置存盘停止
    //通道一 I路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_1_1", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道一 Q路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_1_2", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二 I路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_2_1", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二 Q路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_2_2", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    //设置送数停止
    //通道一
    singleCmdHelper.packSingleCommand("StepGSJDSS_Stop_1", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二
    singleCmdHelper.packSingleCommand("StepGSJDSS_Stop_2", packCommand, masterGSJDDeviceID);
    waitExecSuccess(packCommand);

    // 获取Ka高速基带设备ID 备用
    DeviceID slaveGSJDDeviceID;
    if (!getGSJDDeviceID(slaveGSJDDeviceID, 2))
    {
        return false;
    }

    // 设置存盘停止
    //通道一 I路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_1_1", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道一 Q路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_1_2", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二 I路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_2_1", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二 Q路
    singleCmdHelper.packSingleCommand("StepGSJDCP_Stop_2_2", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    //设置送数停止
    //通道一
    singleCmdHelper.packSingleCommand("StepGSJDSS_Stop_1", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    //通道二
    singleCmdHelper.packSingleCommand("StepGSJDSS_Stop_2", packCommand, slaveGSJDDeviceID);
    waitExecSuccess(packCommand);

    return true;
}

SystemWorkMode XGSResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::XGS; }
