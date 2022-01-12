#include "SZHBHLinkHandler.h"

#include "GlobalData.h"
#include "SingleCommandHelper.h"

SZHBHLinkHandler::SZHBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool SZHBHLinkHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }

    // 数传模式直接返回
    if (SystemWorkModeHelper::isDataTransmissionWorkMode(mLinkLine.workMode))
    {
        return false;
    }

    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!getCKJDDeviceID(ckDeviceID))
    {
        return false;
    }

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

    // 切换前端的工作方式为数字化闭环
    singleCmdHelper.packSingleCommand("StepQD_WORK_SZHBH", packCommand, ckqdDeviceID);
    waitExecSuccess(packCommand);

    // 只用下发一次光口的配置
    setLightNetConfigCKJD_CKQD(ckDeviceID, ckqdDeviceID);

    // 切换S频段射频开关矩阵 模拟源信号输入给前端
    //            singleCmdHelper.packSingleCommand(QString("StepSSPKGJZ_SPMNY_%1").arg(ckqdDeviceID.extenID), packCommand);
    //            waitExecSuccess(packCommand);

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        // 工作方式设置成数字化有线
        singleCmdHelper.packSingleCommand("StepSTTC_SZHBH", packCommand, ckDeviceID);
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
        // 工作方式设置成数字化有线
        singleCmdHelper.packSingleCommand("StepSkuo2_SZHBH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);

        //上行载波输出
        singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级
    else if (mLinkLine.workMode == SYTHSMJ)
    {
        // 工作方式设置成数字化有线
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_SZHBH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        //上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星
    else if (mLinkLine.workMode == SYTHWX)
    {
        // 工作方式设置成数字化有线
        singleCmdHelper.packSingleCommand("StepSYTHWX_SZHBH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        //上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHWX_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率
    else if (mLinkLine.workMode == SYTHGML)
    {
        // 工作方式设置成数字化有线
        singleCmdHelper.packSingleCommand("StepSYTHGML_SZHBH", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
        //上行载波输出
        singleCmdHelper.packSingleCommand("StepSYTHGML_UplinkCarrierOutput_DT", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二共载波
    else if (mLinkLine.workMode == SYTHSMJK2GZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_SZHBH_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_SZHBH_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
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
            // 工作方式设置成(扩频数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_SZHBH_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_SZHBH_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
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
            // 工作方式设置成(扩频数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_SZHBH_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_SZHBH_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
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
            // 工作方式设置成(扩频数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_SZHBH_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_UplinkCarrierOutput_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_SZHBH_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
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
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSKT_SZHBH_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传数字化有线闭环)
            singleCmdHelper.packSingleCommand("StepSKT_SZHBH_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
            //上行载波输出
            singleCmdHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg("扩跳（配置错误）");
        }
    }

    return true;
}

LinkType SZHBHLinkHandler::getLinkType() { return LinkType::SZHBH; }
