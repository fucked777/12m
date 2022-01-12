#include "BaseLinkHandler.h"

#include "DevProtocolSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "LinkConfigHelper.h"
#include "LocalCommunicationAddr.h"
#include "PacketHandler.h"
#include "ResourceRestructuringMessageSerialize.h"
#include "SingleCommandHelper.h"

#include "ConfigMacroMessageSerialize.h"

BaseLinkHandler::BaseLinkHandler(QObject* parent)
    : BaseHandler(parent)
{
}

bool BaseLinkHandler::setLightNetConfigCKJD_CKQD(const DeviceID& ckjdDeviceID, const DeviceID& ckqdDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4424")
    {
        // 测控基带A <---> 测控前端1  测控基带B <---> 测控前端2
        if (ckjdDeviceID.extenID == ckqdDeviceID.extenID)
        {
            singleCmdHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD1", packCommand);
            waitExecSuccess(packCommand);
            singleCmdHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD2", packCommand);
            waitExecSuccess(packCommand);

            singleCmdHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD1", packCommand);
            waitExecSuccess(packCommand);
            singleCmdHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD2", packCommand);
            waitExecSuccess(packCommand);
        }
        // 测控基带A <---> 测控前端2  测控基带B <---> 测控前端1
        else
        {
            singleCmdHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD2", packCommand);
            waitExecSuccess(packCommand);
            singleCmdHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD1", packCommand);
            waitExecSuccess(packCommand);

            singleCmdHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD2", packCommand);
            waitExecSuccess(packCommand);
            singleCmdHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD1", packCommand);
            waitExecSuccess(packCommand);
        }
    }
    else if (curProject == "4426")
    {
        singleCmdHelper.packSingleCommand("StepNet_CKJD1_NewTO_CKQD1", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_CKJD11_NewTO_CKQD1", packCommand);
        waitExecSuccess(packCommand);

        singleCmdHelper.packSingleCommand("StepNet_CKJD2_NewTO_CKQD2", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_CKJD22_NewTO_CKQD2", packCommand);
        waitExecSuccess(packCommand);

        singleCmdHelper.packSingleCommand("StepNet_CKQD1_NewTO_CKJD1", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_CKQD2_NewTO_CKJD2", packCommand);
        waitExecSuccess(packCommand);
    }
    return true;
}

bool BaseLinkHandler::setLightNetConfigDSJD_CKQD(const DeviceID& dsjdDeviceID, const DeviceID& ckqdDeviceID)
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;

    // 低速基带A <---> 测控前端3  低速基带B <---> 测控前端4
    if ((dsjdDeviceID.extenID == 1 && ckqdDeviceID.extenID == 3) || (dsjdDeviceID.extenID == 2 && ckqdDeviceID.extenID == 4))
    {
        singleCmdHelper.packSingleCommand("StepNet_DSJD1_TO_CKQD3", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_DSJD2_TO_CKQD4", packCommand);
        waitExecSuccess(packCommand);

        singleCmdHelper.packSingleCommand("StepNet_CKQD3_TO_DSJD1", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_CKQD4_TO_DSJD2", packCommand);
        waitExecSuccess(packCommand);
    }
    else  // 低速基带A <---> 测控前端4  低速基带B <---> 测控前端3
    {
        singleCmdHelper.packSingleCommand("StepNet_DSJD1_TO_CKQD4", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_DSJD2_TO_CKQD3", packCommand);
        waitExecSuccess(packCommand);

        singleCmdHelper.packSingleCommand("StepNet_CKQD3_TO_DSJD2", packCommand);
        waitExecSuccess(packCommand);
        singleCmdHelper.packSingleCommand("StepNet_CKQD4_TO_DSJD1", packCommand);
        waitExecSuccess(packCommand);
    }
    return true;
}

void BaseLinkHandler::execLinkCommand()
{
    // 获取当前链路模式的配置信息
    const auto& configMacroData = mManualMsg.configMacroData;
    auto configMacroCmdList = configMacroData.configMacroCmdModeMap[mLinkLine.workMode];

    // 配置宏参数命令 组装成QSet，格式为：STTC_LAN_L_SUM:1
    QSet<QString> configMacroCmdParamSet;
    for (auto& configMacroCmd : configMacroCmdList.configMacroCmdMap)
    {
        auto temp = QString("%1:%2").arg(configMacroCmd.id, configMacroCmd.value.toString());
        configMacroCmdParamSet.insert(temp);
    }

    // qWarning() << configMacroCmdParamSet;

    QString json;
    json << configMacroData;
    // qWarning().noquote() << json;

    QList<PackCommand> packCmdList;
    QString errorMsg;

    // 根据配置信息进行匹配链路配置文件命令
    auto flag =
        LinkConfigHelper::getInstance().packLinkConfigCmd(mLinkLine.linkType, mLinkLine.workMode, configMacroCmdParamSet, packCmdList, errorMsg);
    if (flag == false)
    {
        if (!errorMsg.isEmpty())
        {
            emit signalErrorMsg(QString("组包链路配置命令错误：%1").arg(errorMsg));
            return;
        }
    }

    for (auto& cmd : packCmdList)
    {
        waitExecSuccess(cmd);
    }
}
