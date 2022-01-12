#include "OtherResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "UpLinkHandler.h"

OtherResourceReleaseHandler::OtherResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}

bool OtherResourceReleaseHandler::handle()
{
    cleanCZTask();
    cleanACUTask();
    cleanDTETask();
    stopTestDevice();
    return true;
}
SystemWorkMode OtherResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }

void OtherResourceReleaseHandler::cleanCZTask()
{
    if (!mManualMsg.resourceReleaseDeleteCZTask)
    {
        return;
    }

    /* 两台存转直接清空任务就行 */
    mSingleCommandHelper.packSingleCommand("Step_CZJHQK_1", mPackCommand);
    waitExecSuccess(mPackCommand);

    mSingleCommandHelper.packSingleCommand("Step_CZJHQK_2", mPackCommand);
    waitExecSuccess(mPackCommand);
}
void OtherResourceReleaseHandler::cleanACUTask()
{
    if (!mManualMsg.resourceReleaseDeleteACUTask)
    {
        return;
    }

    //获取基带的在线状态和本控状态
    auto onlineACUResult = BaseHandler::getOnlineACU();
    if (!onlineACUResult)
    {
        return;
    }
    auto onlineACU = onlineACUResult.value();

    /* ACU要停止任务
     * 取消自动化运行
     * 设置为待机
     *
     * 如果配置为转收藏 需要转收藏点
     */

    /* 判断处于自动化运行就停任务 1人工 2自动 */
    auto acu = GlobalData::getReportParamData(onlineACU, 1, "TeleDataCode").toInt();
    if (acu == 2)
    {
        /* 任务结束 */
        mSingleCommandHelper.packSingleCommand("Step_ACU_TASK_STOP", mPackCommand, onlineACU);
        waitExecSuccess(mPackCommand);
    }

    /* 人工运行 */
    mSingleCommandHelper.packSingleCommand("Step_ACU_OperatMode_Artificial", mPackCommand, onlineACU);
    waitExecSuccess(mPackCommand);

    /* 待机 */
    mSingleCommandHelper.packSingleCommand("Step_ACU_DaiJI", mPackCommand, onlineACU);
    waitExecSuccess(mPackCommand);

    /* 停止送数 */
    mSingleCommandHelper.packSingleCommand("Step_ACU_2_DET_END", mPackCommand, onlineACU);
    waitExecSuccess(mPackCommand);

//    /* 任务结束是否转收藏 20211203 改为在跟踪结束时间开始收藏 */
//    auto taskEndACUCollection = GlobalData::getAutorunPolicyData("TaskEndACUCollection", "false").toBool();
//    if (taskEndACUCollection)
//    {
//        mSingleCommandHelper.packSingleCommand("Step_ACU_SHOUCANG", mPackCommand, onlineACU);
//        waitExecSuccess(mPackCommand);
//    }
}

void OtherResourceReleaseHandler::stopTestDevice()
{
    UpLinkHandler upLinkHandler;
    upLinkHandler.setEnableHpa(EnableMode::Disable);
    upLinkHandler.setEnableJD(EnableMode::Ignore);
    upLinkHandler.setEnableQD(EnableMode::Ignore);
    upLinkHandler.enableControl(UpLinkDeviceControl::All);
    upLinkHandler.setIgnoreOtherDevice(false);
    upLinkHandler.setIgnoreXLDevice(false);
    upLinkHandler.setManualMessage(mManualMsg);
    upLinkHandler.setRunningFlag(mRunningFlag);

    connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &OtherResourceReleaseHandler::signalSendToDevice);
    connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &OtherResourceReleaseHandler::signalInfoMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &OtherResourceReleaseHandler::signalWarningMsg);
    connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &OtherResourceReleaseHandler::signalErrorMsg);

    upLinkHandler.handle();
}
