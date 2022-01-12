#include "PKXLBPQBHLinkHandler.h"

#include "GlobalData.h"
#include "RWFSLinkHandler.h"
#include "SingleCommandHelper.h"
#include "SystemWorkMode.h"

PKXLBPQBHLinkHandler::PKXLBPQBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool PKXLBPQBHLinkHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 不是测控模式直接跳过
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return false;
    }
    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }

    // 使用主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    // auto masterTargetWorkMode = masterTargetInfo.workMode;

    // 获取S高功放设备ID
    DeviceID sggfDeviceID;
    if (!getSGGFDeviceID(sggfDeviceID))
    {
        return false;
    }

    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }

    // 从配置宏中获取测控前端
    DeviceID ckqdDeviceID;
    if (!getCKQDDeviceID(ckqdDeviceID))
    {
        return false;
    }

    // 切换前端的工作方式为任务方式
    singleCmdHelper.packSingleCommand("StepQD_WORK_RWFS", packCommand, ckqdDeviceID);
    waitExecSuccess(packCommand);

    // 设置功放功率 校零默认使用40dbm 可以在配置中修改
    singleCmdHelper.packSingleCommand("Step_SHPA_TransmPower", packCommand, sggfDeviceID);
    waitExecSuccess(packCommand);

    // 设置功放功率输出
    singleCmdHelper.packSingleCommand("Step_SHPA_Power_Allow", packCommand, sggfDeviceID);
    waitExecSuccess(packCommand);

    // 设置切换开关激励关
    singleCmdHelper.packSingleCommand("StepGFQHKG_JL_ON", packCommand);
    waitExecSuccess(packCommand);

    // 校零是先执行任务环路 等于 先执行配置宏
    //    LinkLine taskLinkLine = mLinkLine;
    //    taskLinkLine.linkType = RWFS;
    //    RWFSLinkHandler handler;
    //    connect(&handler, &BaseLinkHandler::signalSendToDevice, this, &PKXLBPQBHLinkHandler::signalSendToDevice);
    //    connect(&handler, &BaseLinkHandler::signalInfoMsg, this, &PKXLBPQBHLinkHandler::signalInfoMsg);
    //    connect(&handler, &BaseLinkHandler::signalWarningMsg, this, &PKXLBPQBHLinkHandler::signalWarningMsg);
    //    connect(&handler, &BaseLinkHandler::signalErrorMsg, this, &PKXLBPQBHLinkHandler::signalErrorMsg);
    //    handler.setLinkLine(taskLinkLine);
    //    if (!handler.handle())
    //    {
    //        return false;
    //    }

    // 获取上行和下行的频率
    QVariant upFrequency;
    MacroCommon::getUpFrequency(masterTargetTaskCode, mLinkLine.workMode, masterTargetPointFreqNo, upFrequency);
    QVariant downFrequency;
    MacroCommon::getDownFrequency(masterTargetTaskCode, mLinkLine.workMode, masterTargetPointFreqNo, downFrequency);

    QMap<QString, QVariant> frequencyMap;
    frequencyMap["XLFCInputFreq"] = upFrequency;
    frequencyMap["XLFCOutFreq"] = downFrequency;

    // S射频开关网络下发 校零命令
    singleCmdHelper.packSingleCommand("Step_XL", packCommand, frequencyMap);
    waitExecSuccess(packCommand);

    // S射频开关网络 加电
    singleCmdHelper.packSingleCommand("StepSSPKGWL_ON_POWER", packCommand);
    waitExecSuccess(packCommand);

    //    // 宽带测试开关网络 去电 不去电可能有信号干扰
    //    singleCmdHelper.packSingleCommand("StepKDCSKGWL_OFF_POWER", packCommand);
    //    waitExecSuccess(packCommand);

    // 切换基带的工作模式
    if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("%1 切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
        return false;
    }

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        // 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSTTC_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //关闭遥控
        singleCmdHelper.packSingleCommand("StepMACBDC_YK_DOWN", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        //打开测距
        singleCmdHelper.packSingleCommand("StepMACBDC_CJ_UP", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 上行载波输出
        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // 捕获方式设置为自动
        singleCmdHelper.packSingleCommand("StepSTTC_BH_AUTO", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        QThread::sleep(2);

        // 双捕开始
        singleCmdHelper.packSingleCommand("StepSTTC_SB_START", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        // 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSkuo2_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

        // 设置基带上行载波输出
        singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级
    else if (mLinkLine.workMode == SYTHSMJ)
    {
        // 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

        // 设置基带上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星
    else if (mLinkLine.workMode == SYTHWX)
    {
        // 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSYTHWX_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

        // 设置基带上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHWX_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率
    else if (mLinkLine.workMode == SYTHGML)
    {
        // 工作方式设置成偏馈校零变频器
        singleCmdHelper.packSingleCommand("StepSYTHGML_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

        // 设置基带上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHGML_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二共载波
    else if (mLinkLine.workMode == SYTHSMJK2GZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_XL_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_XL_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg("一体化上面级+扩二共载波（配置错误）");
        }
    }
    // 一体化上面级+扩二不共载波
    else if (mLinkLine.workMode == SYTHSMJK2BGZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_XL_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_XL_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg("一体化上面级+扩二不共载波（配置错误）");
        }
    }
    // 一体化卫星+扩二
    else if (mLinkLine.workMode == SYTHWXSK2)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_XL_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_XL_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg(" 一体化卫星+扩二（配置错误）");
        }
    }
    // 一体化高码率+扩二
    else if (mLinkLine.workMode == SYTHGMLSK2)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_XL_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传偏馈校零变频器闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_XL_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);

            // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

            // 设置基带上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg(" 一体化高码率+扩二（配置错误）");
        }
    }
    // 扩跳
    else if (mLinkLine.workMode == SKT)
    {
        singleCmdHelper.packSingleCommand("StepSKT_XL", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        // TODO 需要实时获取当前基带的输出电平，或者从参数宏中获取 OutputLev

        // 设置基带上行载波输出
        singleCmdHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }

    // 下发测控基带功率上天线
    singleCmdHelper.packSingleCommand("Step_GLSTX", packCommand, ckDeviceID);
    waitExecSuccess(packCommand);

    return true;
}

LinkType PKXLBPQBHLinkHandler::getLinkType() { return LinkType::PKXLBPQBH; }
