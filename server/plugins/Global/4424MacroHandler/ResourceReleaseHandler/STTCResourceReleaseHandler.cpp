#include "STTCResourceReleaseHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "SingleCommandHelper.h"

STTCResourceReleaseHandler::STTCResourceReleaseHandler(QObject* parent)
    : BaseResourceReleaseHandler(parent)
{
}

bool STTCResourceReleaseHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 获取S高功放设备ID
    DeviceID sggfDeviceID;
    if (!getSGGFDeviceID(sggfDeviceID))
    {
        return false;
    }
    // 设置S功放去电
    singleCmdHelper.packSingleCommand("Step_SHPA_Power_Forbid", packCommand, sggfDeviceID);
    waitExecSuccess(packCommand);

    // 设置切换开关激励关
    singleCmdHelper.packSingleCommand("StepGFQHKG_JL_OFF", packCommand);
    waitExecSuccess(packCommand);

    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        // 上行载波禁止
        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_TTC", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_TTC", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        // 设置基带上行载波输出禁止
        singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        // 通道一
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_1", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道二
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_2", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道三
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_3", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道四
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_Skuo2_4", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        // 通道一
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_1", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道二
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_2", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道三
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_3", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        // 通道四
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_Skuo2_4", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级
    else if (mLinkLine.workMode == SYTHSMJ)
    {
        // 设置基带上行载波输出禁止
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHSMJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHSMJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星
    else if (mLinkLine.workMode == SYTHWX)
    {
        // 设置基带上行载波输出禁止
        singleCmdHelper.packSingleCommand("StepSYTHWX_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHWX", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHWX", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率
    else if (mLinkLine.workMode == SYTHGML)
    {
        // 设置基带上行载波输出禁止
        singleCmdHelper.packSingleCommand("StepSYTHGML_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHGML", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHGML", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二共载波
    else if (mLinkLine.workMode == SYTHSMJK2GZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_UplinkCarrierOutput_KP_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg("一体化上面级+扩二共载波（配置错误）");
        }

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHSMJK2GZB_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHSMJK2GZB_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHSMJK2GZB_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHSMJK2GZB_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二不共载波
    else if (mLinkLine.workMode == SYTHSMJK2BGZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_UplinkCarrierOutput_KP_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg("一体化上面级+扩二不共载波（配置错误）");
        }

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHSMJK2BGZB_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHSMJK2BGZB_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHSMJK2BGZB_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHSMJK2BGZB_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星+扩二
    else if (mLinkLine.workMode == SYTHWXSK2)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_UplinkCarrierOutput_KP_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg(" 一体化卫星+扩二（配置错误）");
        }

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHWXSK2_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHWXSK2_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHWXSK2_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHWXSK2_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率+扩二
    else if (mLinkLine.workMode == SYTHGMLSK2)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_UplinkCarrierOutput_KP_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 设置基带上行载波输出禁止
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_UplinkCarrierOutput_DT_STOP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg(" 一体化高码率+扩二（配置错误）");
        }

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHGMLSK2_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SYTHGMLSK2_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHGMLSK2_YTH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SYTHGMLSK2_KP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 扩跳
    else if (mLinkLine.workMode == SKT)
    {
        // 设置基带上行载波输出禁止
        singleCmdHelper.packSingleCommand("StepSKT_UplinkCarrierOutput_STOP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 设置存盘停止
        singleCmdHelper.packSingleCommand("StepCKJDCP_Stop_SKT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //设置送数停止
        singleCmdHelper.packSingleCommand("StepCKJDSS_Stop_SKT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }

    // 测控基带功率上天线禁止
    singleCmdHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, ckDeviceID);
    waitExecSuccess(packCommand);

    return true;
}

SystemWorkMode STTCResourceReleaseHandler::getSystemWorkMode() { return SystemWorkMode::STTC; }
