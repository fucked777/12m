#include "BaseLinkHandler.h"

#include "DevProtocolSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "LinkConfigHelper.h"
#include "LocalCommunicationAddr.h"
#include "PacketHandler.h"
#include "ResourceRestructuringMessageSerialize.h"

#include "ConfigMacroMessageSerialize.h"

BaseLinkHandler::BaseLinkHandler(QObject* parent)
    : BaseHandler(parent)
{
}
#if 0
bool BaseLinkHandler::setLightNetConfigCKJD_CKQD(const DeviceID& ckjdDeviceID, const DeviceID& ckqdDeviceID, bool isDoubleMode)
{
    PackCommand packCommand;
    clearQueue();

    /*
     * 20211010
     * 测控基带A 光口网络是192.20.1.111/192.20.2.111
     * 测控基带B 光口网络是192.20.1.112/192.20.2.112
     * 测控前端A 光口网络是192.20.1.101/192.20.2.101
     * 测控前端B 光口网络是192.20.1.102/192.20.2.102
     *
     * StepNet_CKJD1_TO_CKQD1 1 To 1
     * StepNet_CKJD2_TO_CKQD2 2 To 2
     * StepNet_CKJD1_TO_CKQD2 1 To 2
     * StepNet_CKJD2_TO_CKQD1 2 To 1
     *
     * StepNet_CKQD1_TO_CKJD1 1 To 1
     * StepNet_CKQD2_TO_CKJD2 2 To 2
     * StepNet_CKQD1_TO_CKJD2 1 To 2
     * StepNet_CKQD2_TO_CKJD1 2 To 1
     *
     * 因为双模式的存在前端和基带的分机号无法一一对应了
     * 1 2 对应基带A
     * 3 4 对应基带B
     * 这里传入的测控基带分机号理论上只能是1或者3
     *
     * 20211015 这里本来是很正常的 但是当模式为Ka扩2时
     * 他的ID改变了
     */

    /* 如果是双模式的测控基带则需要设置两次光口网络，因为它有两个软件对应两个设备ID */
    // 测控基带A <---> 测控前端1  测控基带B <---> 测控前端2
    if (ckjdDeviceID.extenID == ckqdDeviceID.extenID || ckjdDeviceID.extenID == ckqdDeviceID.extenID + 1)
    {
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD1", packCommand);
        appendExecQueue(packCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD2", packCommand);
        appendExecQueue(packCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD1", packCommand);
        appendExecQueue(packCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD2", packCommand);
        appendExecQueue(packCommand);

        if (isDoubleMode)
        {
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD1Double_TO_CKQD1", packCommand);
            appendExecQueue(packCommand);
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD2Double_TO_CKQD2", packCommand);
            appendExecQueue(packCommand);
        }
    }
    // 测控基带A <---> 测控前端2  测控基带B <---> 测控前端1
    else
    {
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD2", packCommand);
        appendExecQueue(packCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD1", packCommand);
        appendExecQueue(packCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD2", packCommand);
        appendExecQueue(packCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD1", packCommand);
        appendExecQueue(packCommand);

        if (isDoubleMode)
        {
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD1Double_TO_CKQD2", packCommand);
            appendExecQueue(packCommand);
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD2Double_TO_CKQD1", packCommand);
            appendExecQueue(packCommand);
        }
    }
    execQueue();
    return true;
}
#endif
bool BaseLinkHandler::setLightNetConfigCKJD_CKQD(const CKJDControlInfo& info, const DeviceID& ckqdDeviceID, bool isDoubleMode)
{
    /*
     * 20211010
     * 测控基带A 光口网络是192.20.1.111/192.20.2.111
     * 测控基带B 光口网络是192.20.1.112/192.20.2.112
     * 测控前端A 光口网络是192.20.1.101/192.20.2.101
     * 测控前端B 光口网络是192.20.1.102/192.20.2.102
     *
     * StepNet_CKJD1_TO_CKQD1 1 To 1
     * StepNet_CKJD2_TO_CKQD2 2 To 2
     * StepNet_CKJD1_TO_CKQD2 1 To 2
     * StepNet_CKJD2_TO_CKQD1 2 To 1
     *
     * StepNet_CKQD1_TO_CKJD1 1 To 1
     * StepNet_CKQD2_TO_CKJD2 2 To 2
     * StepNet_CKQD1_TO_CKJD2 1 To 2
     * StepNet_CKQD2_TO_CKJD1 2 To 1
     *
     * 因为双模式的存在前端和基带的分机号无法一一对应了
     * 1 2 对应基带A
     * 3 4 对应基带B
     * 这里传入的测控基带分机号理论上只能是1或者3
     *
     * 20211015 这里本来是很正常的 但是当模式为Ka扩2时
     * 他的ID改变了 拿测控1举例在STTC SKUO2 skt时 ID是4001 但是ka扩2时ID变成了4002
     */
    /* 如果是双模式的测控基带则需要设置两次光口网络，因为它有两个软件对应两个设备ID */

    bool isJDA = (info.afterMaster.extenID == 1 || info.afterMaster.extenID == 2) || (info.afterSlave.extenID == 3 || info.afterSlave.extenID == 4);
    bool isQDA = ckqdDeviceID.extenID == 1;


    clearQueue();
    // 测控基带A <---> 测控前端1  测控基带B <---> 测控前端2
    if (isJDA == isQDA)
    {
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD1", mPackCommand, isJDA ? info.afterMaster : info.afterSlave);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD2", mPackCommand, isJDA ? info.afterSlave : info.afterMaster);
        appendExecQueue(mPackCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD1", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD2", mPackCommand);
        appendExecQueue(mPackCommand);

        if (isDoubleMode)
        {
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD1Double_TO_CKQD1", mPackCommand);
            waitExecSuccess(mPackCommand);
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD2Double_TO_CKQD2", mPackCommand);
            waitExecSuccess(mPackCommand);
        }
    }
    // 测控基带A <---> 测控前端2  测控基带B <---> 测控前端1
    else
    {
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD1_TO_CKQD2", mPackCommand, isJDA ? info.afterMaster : info.afterSlave);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKJD2_TO_CKQD1", mPackCommand, isJDA ? info.afterSlave : info.afterMaster);
        appendExecQueue(mPackCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD1_TO_CKJD2", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD2_TO_CKJD1", mPackCommand);
        appendExecQueue(mPackCommand);

        if (isDoubleMode)
        {
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD1Double_TO_CKQD2", mPackCommand);
            waitExecSuccess(mPackCommand);
            mSingleCommandHelper.packSingleCommand("StepNet_CKJD2Double_TO_CKQD1", mPackCommand);
            waitExecSuccess(mPackCommand);
        }
    }

    execQueue();

    return true;
}
bool BaseLinkHandler::setLightNetConfigDSJD_CKQD(const DeviceID& dsjdDeviceID, const DeviceID& ckqdDeviceID)
{
    clearQueue();
    // 低速基带A <---> 测控前端3  低速基带B <---> 测控前端4
    if ((dsjdDeviceID.extenID == 1 && ckqdDeviceID.extenID == 3) || (dsjdDeviceID.extenID == 2 && ckqdDeviceID.extenID == 4))
    {
        mSingleCommandHelper.packSingleCommand("StepNet_DSJD1_TO_CKQD3", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_DSJD2_TO_CKQD4", mPackCommand);
        appendExecQueue(mPackCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD3_TO_DSJD1", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD4_TO_DSJD2", mPackCommand);
        appendExecQueue(mPackCommand);
    }
    else  // 低速基带A <---> 测控前端4  低速基带B <---> 测控前端3
    {
        mSingleCommandHelper.packSingleCommand("StepNet_DSJD1_TO_CKQD4", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_DSJD2_TO_CKQD3", mPackCommand);
        appendExecQueue(mPackCommand);

        mSingleCommandHelper.packSingleCommand("StepNet_CKQD3_TO_DSJD2", mPackCommand);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepNet_CKQD4_TO_DSJD1", mPackCommand);
        appendExecQueue(mPackCommand);
    }
    execQueue();
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

    // QString json;
    // json << configMacroData;
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

    clearQueue();
    for (auto& cmd : packCmdList)
    {
        appendExecQueue(cmd);
    }
    execQueue();
}
void BaseLinkHandler::execLinkCommand(const QSet<SystemWorkMode>&workModeSet)
{
    // 获取当前链路模式的配置信息
    const auto& configMacroData = mManualMsg.configMacroData;

    // 配置宏参数命令 组装成QSet，格式为：STTC_LAN_L_SUM:1
    QSet<QString> configMacroCmdParamSet;
    for(auto&item : workModeSet)
    {
        auto configMacroCmdList = configMacroData.configMacroCmdModeMap.value(item);

        for (auto& configMacroCmd : configMacroCmdList.configMacroCmdMap)
        {
            auto temp = QString("%1:%2").arg(configMacroCmd.id, configMacroCmd.value.toString());
            configMacroCmdParamSet.insert(temp);
        }
    }

    QList<PackCommand> packCmdList;
    QString errorMsg;

    // 根据配置信息进行匹配链路配置文件命令
    auto flag =
        LinkConfigHelper::getInstance().packLinkConfigCmd(mLinkLine.linkType, workModeSet, configMacroCmdParamSet, packCmdList, errorMsg);
    if (flag == false)
    {
        if (!errorMsg.isEmpty())
        {
            emit signalErrorMsg(QString("组包链路配置命令错误：%1").arg(errorMsg));
            return;
        }
    }

    clearQueue();
    for (auto& cmd : packCmdList)
    {
        appendExecQueue(cmd);
    }
    execQueue();
}

bool BaseLinkHandler::ckqdLink(CKQDMode mode)
{
    // 从配置宏中获取测控前端
    auto result = getCKQDDeviceID();
    if (!result)
    {
        emit signalErrorMsg(result.msg());
        return false;
    }
    m_ckqdDeviceInfo = result.value();

    clearQueue();
    // 切换前端的工作方式为数字化闭环
    mSingleCommandHelper.packSingleCommand(mode == CKQDMode::SZHBH ? "StepQD_WORK_SZHBH" : "StepQD_WORK_RWFS", mPackCommand, m_ckqdDeviceInfo.master);
    appendExecQueue(mPackCommand);

    if (m_ckqdDeviceInfo.slave.isValid())
    {
        mSingleCommandHelper.packSingleCommand(mode == CKQDMode::SZHBH ? "StepQD_WORK_SZHBH" : "StepQD_WORK_RWFS", mPackCommand,
                                               m_ckqdDeviceInfo.slave);
        appendExecQueue(mPackCommand);
    }
    execQueue();
    return true;
}
