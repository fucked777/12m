#include "LinkConfigHelper.h"

#include "GlobalData.h"
#include "LinksXmlReader.h"
#include "LocalCommunicationAddr.h"
#include "MessagePublish.h"
#include "PacketHandler.h"
#include "ProcessControlCmdPacker.h"
#include "SystemWorkMode.h"
#include "UnitParamSetPacker.h"

LinkConfigHelper::LinkConfigHelper()
{
    PacketHandler::instance().appendPacker(new ProcessControlCmdPacker());
    PacketHandler::instance().appendPacker(new UnitParamSetPacker());

    // 读取链路配置
    LinksXmlReader reader;
    QString errorMsg;
    auto linkMap = reader.getLinkConfig(errorMsg);
    GlobalData::setLinkConfigData(linkMap);
    if (!errorMsg.isEmpty())
    {
        SystemLogPublish::errorMsg(errorMsg);
    }
}

bool LinkConfigHelper::packLinkConfigCmd(LinkType linkType, SystemWorkMode workMode, const QSet<QString> keyValuePairSet,
                                         QList<PackCommand>& packCmdList, QString& errorMsg)
{
    // 读取链路配置  测试时使用，避免修改链路配置文件需要重新启动服务器
    //    LinksXmlReader reader;
    //    auto linkMap = reader.getLinkConfig(errorMsg);
    //    GlobalData::setLinkConfigData(linkMap);
    //    if (!errorMsg.isEmpty())
    //    {
    //        return false;
    //    }

    // 获取链路配置文件模式信息
    LinkMode linkMode;
    if (!getLinkConfigMode(linkType, workMode, linkMode, errorMsg))
    {
        return false;
    }

    QSet<QString> emptySet;
    emptySet << QString("");
    QSet<QSet<QString>> emptySets;
    emptySets << emptySet;

    QList<Command> commandList;
    for (auto commandGroup : linkMode.cmdGroups)
    {
        // 如果命令组条件为空，表示任何条件都满足，该组下的命令都要发送
        if (commandGroup.conditionSet == emptySets)
        {
            commandList << commandGroup.commands;
            continue;
        }

        for (const auto& condition : commandGroup.conditionSet)
        {
            if (keyValuePairSet.contains(condition))  // 满足条件之一就下发
            {
                commandList << commandGroup.commands;
                break;
            }
        }
    }

    if (commandList.isEmpty())
    {
        errorMsg = QString("配置宏未匹配到对应的链路配置文件");
        return false;
    }

    // 合并相同命令
    QList<Command> mergeCmdList;
    mergeCommands(commandList, mergeCmdList);

    // 组包
    for (auto& command : mergeCmdList)
    {
        PackCommand packCmd;
        if (!packCommand(command, packCmd))
        {
            packCmd.errorMsg = errorMsg;
        }

        packCmdList << packCmd;
    }

    return true;
}


bool LinkConfigHelper::packLinkConfigCmd(LinkType linkType,const  QSet<SystemWorkMode>& workModeSet, const QSet<QString> keyValuePairSet, QList<PackCommand>& packCmdList,
                           QString& errorMsg)
{
    // 获取链路配置文件模式信息
    LinkMode linkMode;
    for(auto&item : workModeSet)
    {
        LinkMode tempLinkMode;
        if (!getLinkConfigMode(linkType, item, tempLinkMode, errorMsg))
        {
            return false;
        }
        linkMode.cmdGroups << tempLinkMode.cmdGroups;
    }


    QSet<QString> emptySet;
    emptySet << QString("");
    QSet<QSet<QString>> emptySets;
    emptySets << emptySet;

    QList<Command> commandList;
    for (auto commandGroup : linkMode.cmdGroups)
    {
        // 如果命令组条件为空，表示任何条件都满足，该组下的命令都要发送
        if (commandGroup.conditionSet == emptySets)
        {
            commandList << commandGroup.commands;
            continue;
        }

        for (const auto& condition : commandGroup.conditionSet)
        {
            if (keyValuePairSet.contains(condition))  // 满足条件之一就下发
            {
                commandList << commandGroup.commands;
                break;
            }
        }
    }

    if (commandList.isEmpty())
    {
        errorMsg = QString("配置宏未匹配到对应的链路配置文件");
        return false;
    }

    // 合并相同命令
    QList<Command> mergeCmdList;
    mergeCommands(commandList, mergeCmdList);

    // 组包
    for (auto& command : mergeCmdList)
    {
        PackCommand packCmd;
        if (!packCommand(command, packCmd))
        {
            packCmd.errorMsg = errorMsg;
        }

        packCmdList << packCmd;
    }

    return true;
}


bool LinkConfigHelper::getLinkConfigMode(LinkType linkType, int modeId, LinkMode& linkMode, QString& errorMsg)
{
    // 获取链路配置文件数据
    LinkConfigMap linkConfigMap;
    if (!GlobalData::getLinkConfigData(linkConfigMap))
    {
        errorMsg = QString("从全局获取链路配置数据失败");
        return false;
    }

    // 获取指定链路
    auto linkConfigIter = linkConfigMap.find(linkType);
    if (linkConfigIter == linkConfigMap.end())
    {
        errorMsg = QString("链路配置文件不包含id为:%1的链路").arg((int)linkType);
        return false;
    }

    // 获取链路下的指定模式
    auto linkModeIter = linkConfigIter.value().modeMap.find(modeId);
    if (linkModeIter == linkConfigIter.value().modeMap.end())
    {
        errorMsg = QString("配置文件id为%1的链路，不包含id为:%2的模式").arg((int)linkType).arg(modeId);
        return false;
    }

    linkMode = linkModeIter.value();
    return true;
}

void LinkConfigHelper::mergeCommands(const QList<Command>& commandList, QList<Command>& mergedCommandList)
{
    // 筛选到一起
    QMap<QString, QList<Command>> cmdMap;
    for (const auto& command : commandList)
    {
        QString key = QString("%1_%2_%3_%4").arg(command.deviceId).arg(command.modeId).arg(static_cast<int>(command.cmdType)).arg(command.cmdId);
        cmdMap[key].append(command);
    }

    // 合并参数
    for (const auto& cmdList : cmdMap)
    {
        auto mergeCommand = cmdList.at(0);
        for (int i = 1; i < cmdList.size(); ++i)
        {
            auto tempCmd = cmdList.at(i);
            for (auto paramId : tempCmd.paramMap.keys())
            {
                mergeCommand.paramMap[paramId] = tempCmd.paramMap[paramId];
            }
        }

        mergedCommandList << mergeCommand;
    }
}

bool LinkConfigHelper::packCommand(const Command& command, PackCommand& packcmd)
{
    // Command 转为 PackCommand
    packcmd.deviceID = DeviceID(command.deviceId);
    packcmd.modeId = command.modeId;
    packcmd.cmdId = command.cmdId;
    packcmd.cmdType = command.cmdType;
    packcmd.operatorInfo = command.operatorInfo;

    // 组包
    PackMessage packMsg;

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();
    packMsg.header.sourceAddr = sourceAddr;

    DeviceID deviceID(command.deviceId);
    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = deviceID.sysID;
    targetAddr.deviceNumb = deviceID.devID;
    targetAddr.extenNumb = deviceID.extenID;
    packMsg.header.targetAddr = targetAddr;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;
    packMsg.header.version = version;

    packMsg.header.msgType = command.cmdType;

    QString erroMsg;

    // 过程控制命令
    if (command.cmdType == DevMsgType::ProcessControlCmd)
    {
        ProcessControlCmdMessage processCtrlCmdMsg;
        processCtrlCmdMsg.mode = command.modeId;
        processCtrlCmdMsg.cmdId = command.cmdId;
        processCtrlCmdMsg.settingParamMap = command.paramMap;
        packMsg.processCtrlCmdMsg = processCtrlCmdMsg;

        // 获取下发的参数名称和对应的值描述
        getProcessControlCmdSettingParamDesc(targetAddr, processCtrlCmdMsg, packcmd.paramDescValueMap);
    }
    // 单元参数设置命令
    else if (command.cmdType == DevMsgType::UnitParameterSetCmd)
    {
        UnitParamSetMessage unitSetMsg;
        unitSetMsg.mode = command.modeId;
        unitSetMsg.unitId = command.cmdId;
        unitSetMsg.channelValidIdent = 0;  // 暂时只支持无多目标的单元参数设置
        unitSetMsg.settingParamMap = command.paramMap;
        packMsg.unitParamSetMsg = unitSetMsg;

        // 获取下发的参数名称和对应的值描述
        getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packcmd.paramDescValueMap);
    }
    else
    {
        packcmd.errorMsg = QString("链路配置命令的类型错误");
        return false;
    }

    if (!PacketHandler::instance().pack(packMsg, packcmd.data, erroMsg))
    {
        packcmd.errorMsg = QString("链路配置命令组包失败");
        return false;
    }

    return true;
}

void LinkConfigHelper::getProcessControlCmdSettingParamDesc(const MessageAddress& msgAddr, const ProcessControlCmdMessage& processCtrlCmdMsg,
                                                            QMap<QString, QVariant>& paramDescValueMap)
{
    // 获取命令参数
    CmdAttribute cmdAttr = GlobalData::getCmd(msgAddr, processCtrlCmdMsg.mode, processCtrlCmdMsg.cmdId);
    auto modeCtrl = GlobalData::getMode(msgAddr, processCtrlCmdMsg.mode);

    for (auto paramAttr : cmdAttr.requestList)
    {
        auto paramValIter = processCtrlCmdMsg.settingParamMap.find(paramAttr.id);
        if (paramValIter != processCtrlCmdMsg.settingParamMap.end())
        {
            auto paramValue = paramValIter.value();
            if (paramAttr.displayFormat == DisplayFormat::DisplayFormat_Enum)
            {
                auto enumEntryList = modeCtrl.cmdEnumMap[paramAttr.enumType].emumEntryList;
                for (auto enumEntry : enumEntryList)
                {
                    if (enumEntry.uValue == paramValue)
                    {
                        paramDescValueMap[paramAttr.desc] = enumEntry.desc;
                        break;
                    }
                }
            }
            else
            {
                paramDescValueMap[paramAttr.desc] = paramValue;
            }
        }
    }
}

void LinkConfigHelper::getUnitParamSetCmdParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg,
                                                   QMap<QString, QVariant>& paramDescValueMap)
{
    // 获取单元参数
    Unit unit = GlobalData::getUnit(msgAddr, unitSetMsg.mode, unitSetMsg.unitId);
    for (auto paramAttr : unit.parameterSetList)
    {
        auto paramValIter = unitSetMsg.settingParamMap.find(paramAttr.id);
        if (paramValIter != unitSetMsg.settingParamMap.end())
        {
            auto groupName = unit.getParamGroupName(paramAttr.id);  // 获取分组名

            auto paramValue = paramValIter.value();
            if (paramAttr.displayFormat == DisplayFormat::DisplayFormat_Enum)
            {
                auto enumEntryList = unit.enumMap[paramAttr.enumType].emumEntryList;
                for (auto enumEntry : enumEntryList)
                {
                    if (enumEntry.uValue == paramValue)
                    {
                        paramDescValueMap[groupName + paramAttr.desc] = enumEntry.desc;
                        break;
                    }
                }
            }
            else
            {
                paramDescValueMap[paramAttr.desc] = paramValue;
            }
        }
    }
}
