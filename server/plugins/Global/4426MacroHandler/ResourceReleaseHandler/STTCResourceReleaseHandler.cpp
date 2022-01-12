#include "STTCResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"

STTCResourceReleaseHandler::STTCResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}
bool STTCResourceReleaseHandler::sggf()
{
    // 设置S功放去电
    mSingleCommandHelper.packSingleCommand("Step_300SHPA_Power_Forbid", mPackCommand);
    appendExecQueue(mPackCommand);
    // 设置S功放去电
    mSingleCommandHelper.packSingleCommand("Step_30SHPA_Power_Forbid", mPackCommand);
    appendExecQueue(mPackCommand);
    // 设置切换开关激励关
    mSingleCommandHelper.packSingleCommand("StepGFQHKG_JL_OFF", mPackCommand);
    appendExecQueue(mPackCommand);

    return true;
}
bool STTCResourceReleaseHandler::kagf()
{
    // 设置Ka遥测功放去电
    mSingleCommandHelper.packSingleCommand("StepKa300HPA_Power_Forbid", mPackCommand);
    appendExecQueue(mPackCommand);
    // 设置Ka遥测功放去电
    mSingleCommandHelper.packSingleCommand("StepKa30HPA_Power_Forbid", mPackCommand);
    appendExecQueue(mPackCommand);

    return true;
}

bool STTCResourceReleaseHandler::sttc(const DeviceID& deviceID)
{
    // 上行载波禁止
    // mSingleCommandHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput_STOP", mPackCommand, deviceID);
    // appendExecQueue(mPackCommand);

    // 设置存盘停止
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_TTC", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    if (GlobalData::getReportParamData(deviceID, 1, "OnLineMachine").toInt() == 1)
    {
        //设置送数停止
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_TTC", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }

    // 测控基带功率上天线禁止
    mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    return false;
}
bool STTCResourceReleaseHandler::skuo2(const DeviceID& deviceID)
{
    // 设置基带上行载波输出禁止
    //    mSingleCommandHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput_STOP", mPackCommand, deviceID);
    //    appendExecQueue(mPackCommand);

    // 设置存盘停止
    // 通道一
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_1", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    // 通道二
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_2", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    // 通道三
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_3", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    // 通道四
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_4", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    if (GlobalData::getReportParamData(deviceID, 1, "OnLineMachine").toInt() == 1)
    {
        //设置送数停止
        // 通道一
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_1", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        // 通道二
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_2", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        // 通道三
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_3", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        // 通道四
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_4", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }

    // 测控基带功率上天线禁止
    mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    return true;
}
bool STTCResourceReleaseHandler::kakuo2(const DeviceID& deviceID)
{
    // 设置基带上行载波输出禁止
    // mSingleCommandHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput_STOP", mPackCommand, deviceID);
    // appendExecQueue(mPackCommand);

    if (GlobalData::getReportParamData(deviceID, 1, "OnLineMachine").toInt() == 1)
    {
        //关闭基带送数
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_KaKuo2_1", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_KaKuo2_2", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_KaKuo2_3", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_KaKuo2_4", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }

    //关闭基带存盘
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_KaKuo2_1", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_KaKuo2_2", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_KaKuo2_3", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_KaKuo2_4", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    // 测控基带功率上天线禁止
    mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    return true;
}
bool STTCResourceReleaseHandler::skt(const DeviceID& deviceID)
{
    // 设置基带上行载波输出禁止
    // mSingleCommandHelper.packSingleCommand("StepSKT_UplinkCarrierOutput_STOP", mPackCommand, deviceID);
    // appendExecQueue(mPackCommand);

    // 设置存盘停止
    mSingleCommandHelper.packSingleCommand("StepCKJDCP_Stop_SKT", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    if (GlobalData::getReportParamData(deviceID, 1, "OnLineMachine").toInt() == 1)
    {
        //设置送数停止
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Stop_SKT", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }

    // 测控基带功率上天线禁止
    mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);

    return true;
}
bool STTCResourceReleaseHandler::handle()
{
    /* 4个设备ID */
    QList<DeviceID> s;
    QList<DeviceID> ka;

    DeviceID deviceID(4, 0, 1);
    if (GlobalData::getDeviceOnline(deviceID))
    {
        s << deviceID;
    }
    deviceID.extenID = 3;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        s << deviceID;
    }

    deviceID.extenID = 2;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        ka << deviceID;
    }

    deviceID.extenID = 4;
    if (GlobalData::getDeviceOnline(deviceID))
    {
        ka << deviceID;
    }

    clearQueue();

    /* 功放 */
    sggf();
    kagf();

    for (auto& item : s)
    {
        auto workMode = GlobalData::getReportParamData(item, 1, "WorkingMode").toInt();
        if (workMode == 0x01)
        {
            sttc(item);
        }
        else if (workMode == 0x04)
        {
            skuo2(workMode);
        }
        else if (workMode == 0x21)
        {
            skt(workMode);
        }
    }
    for (auto& item : ka)
    {
        kakuo2(item);
    }

    execQueue();
    return true;
}

SystemWorkMode STTCResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::STTC; }
