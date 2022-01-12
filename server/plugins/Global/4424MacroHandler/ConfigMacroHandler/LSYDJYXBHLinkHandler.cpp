#include "LSYDJYXBHLinkHandler.h"

#include "GlobalData.h"
#include "SingleCommandHelper.h"
#include "SystemWorkMode.h"

LSYDJYXBHLinkHandler::LSYDJYXBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool LSYDJYXBHLinkHandler::handle()
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

    // 获取测控基带ID
    DeviceID ckDeviceID;
    if (!(getCKJDDeviceID(ckDeviceID)))
    {
        return false;
    }

    // 获取测控前端
    DeviceID ckqdDeviceID;
    if (!getCKQDDeviceID(ckqdDeviceID))
    {
        return false;
    }

    // 进行切换基带的工作模式
    if (!switchJDWorkMode(ckDeviceID, mLinkLine.workMode))
    {
        emit signalErrorMsg(QString("%1 切换模式失败，任务工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(ckDeviceID)));
        return false;
    }

    // 只用下发一次光口的配置
    setLightNetConfigCKJD_CKQD(ckDeviceID, ckqdDeviceID);

    // 设置S频段接收开关矩阵 输出联试应答机信号到测控前端
    if (mLinkLine.workMode == SKT)  // 如果是扩跳模式，从扩跳联试应答机输出
    {
        singleCmdHelper.packSingleCommand(QString("StepSSPKGJZ_LSYDJ_KT_%1").arg(ckqdDeviceID.extenID), packCommand);
        waitExecSuccess(packCommand);
    }
    else  // 其他就从常规联试应答机输出
    {
        singleCmdHelper.packSingleCommand(QString("StepSSPKGJZ_LSYDJ_CG_%1").arg(ckqdDeviceID.extenID), packCommand);
        waitExecSuccess(packCommand);
    }

    // S标准TTC
    if (mLinkLine.workMode == STTC)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepSTTC_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // S扩二
    else if (mLinkLine.workMode == Skuo2)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepSkuo2_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // Ka扩二
    else if (mLinkLine.workMode == KaKuo2)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepKakuo2_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级
    else if (mLinkLine.workMode == SYTHSMJ)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepSYTHSMJ_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星
    else if (mLinkLine.workMode == SYTHWX)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepSYTHWX_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率
    else if (mLinkLine.workMode == SYTHGML)
    {
        // 工作方式设置成联试应答机闭环
        singleCmdHelper.packSingleCommand("StepSYTHGML_LSYDJ", packCommand, ckDeviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二共载波
    else if (mLinkLine.workMode == SYTHSMJK2GZB)
    {
        if (mLinkLine.masterType == YTH_KP)
        {
            // 工作方式设置成(扩频联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_LSYDJ_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2GZB_LSYDJ_DT", packCommand, ckDeviceID);
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
            // 工作方式设置成(扩频联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_LSYDJ_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHSMJK2BGZB_LSYDJ_DT", packCommand, ckDeviceID);
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
            // 工作方式设置成(扩频联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_LSYDJ_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHWXSK2_LSYDJ_DT", packCommand, ckDeviceID);
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
            // 工作方式设置成(扩频联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_LSYDJ_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSYTHGMLSK2_LSYDJ_DT", packCommand, ckDeviceID);
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
            // 工作方式设置成(扩频联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSKT_LSYDJ_KP", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else if (mLinkLine.masterType == YTH_DT)
        {
            // 工作方式设置成(数传联试应答机闭环闭环)
            singleCmdHelper.packSingleCommand("StepSKT_LSYDJ_DT", packCommand, ckDeviceID);
            waitExecSuccess(packCommand);
        }
        else
        {
            emit signalErrorMsg(" 一体化高码率+扩二（配置错误）");
        }
    }

    return true;
}

LinkType LSYDJYXBHLinkHandler::getLinkType() { return LinkType::LSYDJYXBH; }
