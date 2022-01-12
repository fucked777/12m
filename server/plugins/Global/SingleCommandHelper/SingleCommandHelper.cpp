#include "SingleCommandHelper.h"

#include "GlobalData.h"
#include "LocalCommunicationAddr.h"
#include "MessagePublish.h"
#include "PacketHandler.h"
#include "ProcessControlCmdPacker.h"
#include "UnitParamSetPacker.h"

SingleCommandHelper::SingleCommandHelper()
{
    PacketHandler::instance().appendPacker(new ProcessControlCmdPacker());
    PacketHandler::instance().appendPacker(new UnitParamSetPacker());

    SingleCommandXmlReader reader;
    QString errorMsg;
    mSingleCmdMap = reader.getSingleCmdMap(errorMsg);
    if (!errorMsg.isEmpty())
    {
        SystemLogPublish::errorMsg(errorMsg);
    }
}
QVariant SingleCommandHelper::getSingleCmdValue(const QString& stepKey, const QString& filedKey, const QVariant& defaultValue)
{
    return mSingleCmdMap.value(stepKey.toUpper()).paramMap.value(filedKey, defaultValue);
}
DeviceID SingleCommandHelper::getSingleCmdDeviceID(const QString& stepKey, const QString& defaultValue)
{
    auto param = mSingleCmdMap.value(stepKey);
    return param.deviceId.isValid() ? param.deviceId : DeviceID::fromHex(defaultValue);
}
int SingleCommandHelper::getSingleCmdUnitOrCmd(const QString& stepKey, int defaultValue)
{
    auto param = mSingleCmdMap.value(stepKey);
    return param.cmdId <= 0 ? defaultValue : param.cmdId;
}

bool SingleCommandHelper::packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID,
                                            QMap<QString, QVariant> replaceParamMap) const
{
    return packSingleCommand(singleCmdId, packCommand, replaceDeviceID, -1, replaceParamMap);
}

bool SingleCommandHelper::packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, QMap<QString, QVariant> replaceParamMap) const
{
    return packSingleCommand(singleCmdId, packCommand, DeviceID(), -1, replaceParamMap);
}

bool SingleCommandHelper::packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID, int replaceModeId,
                                            QMap<QString, QVariant> replaceParamMap) const
{
    //  重置命令信息
    packCommand.reset();

    /* 20210830 wp?? 不区分大小写了 */
    auto singleCmdIter = mSingleCmdMap.find(singleCmdId.toUpper());
    if (singleCmdIter == mSingleCmdMap.end())
    {
        packCommand.errorMsg = QString("配置错误，未匹配到Id为%1的单命令").arg(singleCmdId);
        return false;
    }

    // 获取配置文件中的命令信息
    auto singleCmd = singleCmdIter.value();

    // 如果替换的设备Id有效，替换配置文件中的设备ID再进行组包
    if (replaceDeviceID.isValid())
    {
        singleCmd.deviceId = replaceDeviceID;
    }

    // 如果模式有效，替换模式id
    if (replaceModeId != -1)
    {
        singleCmd.modeId = replaceModeId;
    }

    // 不存在的参数插入，存在的进行替换参数值
    if (!replaceParamMap.isEmpty())
    {
        for (auto paramId : replaceParamMap.keys())
        {
            singleCmd.paramMap[paramId] = replaceParamMap[paramId];
        }
    }

    // 组包单命令
    return packSingleCommand(singleCmd, packCommand);
}

bool SingleCommandHelper::packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID,
                                            SystemWorkMode replaceSystemMode, QMap<QString, QVariant> replaceParamMap) const
{
    auto modeId = SystemWorkModeHelper::systemWorkModeConverToModeId(replaceSystemMode);
    return packSingleCommand(singleCmdId, packCommand, replaceDeviceID, modeId, replaceParamMap);
}

bool SingleCommandHelper::packSingleCommand(const QString& singleCmdId, PackCommand& packCommand,
                                            const QMap<int, QList<QPair<QString, QVariant>>>& multiParamMap, DeviceID replaceDeviceID,
                                            const QVariantMap& replaceParamMap) const
{
    //  重置命令信息
    packCommand.reset();

    /* 20210830 wp?? 不区分大小写了 */
    auto singleCmdIter = mSingleCmdMap.find(singleCmdId.toUpper());
    if (singleCmdIter == mSingleCmdMap.end())
    {
        packCommand.errorMsg = QString("配置错误，未匹配到Id为%1的单命令").arg(singleCmdId);
        return false;
    }

    // 获取配置文件中的命令信息
    auto singleCmd = singleCmdIter.value();

    // 如果替换的设备Id有效，替换配置文件中的设备ID再进行组包
    if (replaceDeviceID.isValid())
    {
        singleCmd.deviceId = replaceDeviceID;
    }

    // 不存在的参数插入，存在的进行替换参数值
    if (!replaceParamMap.isEmpty())
    {
        for (auto paramId : replaceParamMap.keys())
        {
            singleCmd.paramMap[paramId] = replaceParamMap[paramId];
        }
    }
    singleCmd.multiParamMap = multiParamMap;

    // 组包单命令
    return packSingleCommand(singleCmd, packCommand);
}

bool SingleCommandHelper::packSingleCommand(const SingleCommand& singleCmd, PackCommand& packCommand) const
{
    // 把SingleCommand 转为 PackCommand
    packCommand.deviceID = singleCmd.deviceId;
    packCommand.modeId = singleCmd.modeId;
    packCommand.cmdId = singleCmd.cmdId;
    packCommand.cmdType = singleCmd.cmdType;
    packCommand.operatorInfo = singleCmd.deviceInfo;

    // 组包
    PackMessage packMsg;

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();
    packMsg.header.sourceAddr = sourceAddr;

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = singleCmd.deviceId.sysID;
    targetAddr.deviceNumb = singleCmd.deviceId.devID;
    targetAddr.extenNumb = singleCmd.deviceId.extenID;
    packMsg.header.targetAddr = targetAddr;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;
    packMsg.header.version = version;

    packMsg.header.msgType = singleCmd.cmdType;

    // 过程控制命令
    if (singleCmd.cmdType == DevMsgType::ProcessControlCmd)
    {
        ProcessControlCmdMessage processCtrlCmdMsg;
        processCtrlCmdMsg.mode = singleCmd.modeId;
        processCtrlCmdMsg.cmdId = singleCmd.cmdId;
        processCtrlCmdMsg.settingParamMap = singleCmd.paramMap;
        processCtrlCmdMsg.multiParamMap = singleCmd.multiParamMap;
        packMsg.processCtrlCmdMsg = processCtrlCmdMsg;

        // 获取下发的参数名称和对应的值描述
        getProcessControlCmdSettingParamDesc(targetAddr, processCtrlCmdMsg, packCommand.paramDescValueMap);
    }
    // 单元参数设置命令
    else if (singleCmd.cmdType == DevMsgType::UnitParameterSetCmd)
    {
        UnitParamSetMessage unitSetMsg;
        /* 新加的类型处理多目标的 */
        if (singleCmd.cmdTypeRaw == 3)
        {
            if (!createMultiPackMsg(targetAddr, singleCmd, packCommand, packMsg))
            {
                return false;
            }
        }
        else
        {
            if (!createPackMsg(targetAddr, singleCmd, packCommand, packMsg))
            {
                return false;
            }
        }
    }
    else
    {
        packCommand.errorMsg = QString("单命令命令类型错误,请检查配置文件");
        return false;
    }

    QString errorMsg;
    if (!PacketHandler::instance().pack(packMsg, packCommand.data, errorMsg))
    {
        packCommand.errorMsg = QString("单命令%1组包失败：%2").arg(singleCmd.groupName).arg(errorMsg);
        return false;
    }

    return true;
}

void SingleCommandHelper::getProcessControlCmdSettingParamDesc(const MessageAddress& msgAddr, const ProcessControlCmdMessage& processCtrlCmdMsg,
                                                               QMap<QString, QVariant>& paramDescValueMap) const
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

void SingleCommandHelper::getUnitParamSetCmdParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg,
                                                      QMap<QString, QVariant>& paramDescValueMap) const
{
    // 获取单元参数
    Unit unit = GlobalData::getUnit(msgAddr, unitSetMsg.mode, unitSetMsg.unitId);
    for (auto paramAttr : unit.parameterSetList)
    {
        auto paramValIter = unitSetMsg.settingParamMap.find(paramAttr.id);
        if (paramValIter != unitSetMsg.settingParamMap.end())
        {
            auto groupName = unit.getParamGroupName(paramAttr.id);  // 获取分组名
            auto key = groupName.isEmpty() ? paramAttr.desc : QString("%1:%2").arg(groupName, paramAttr.desc);

            auto paramValue = paramValIter.value();
            if (paramAttr.displayFormat == DisplayFormat::DisplayFormat_Enum)
            {
                auto enumEntryList = unit.enumMap[paramAttr.enumType].emumEntryList;
                for (auto enumEntry : enumEntryList)
                {
                    if (enumEntry.uValue == paramValue)
                    {
                        paramDescValueMap[key] = enumEntry.desc;
                        break;
                    }
                }
            }
            else
            {
                paramDescValueMap[key] = paramValue;
            }
        }
    }
}
void SingleCommandHelper::getUnitParamSetCmdMultiParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg,
                                                           const QMap<QString, QVariant>& src, QMap<QString, QVariant>& paramDescValueMap,
                                                           int targetNo, bool fixed) const
{
    // 获取单元参数
    Unit unit = GlobalData::getUnit(msgAddr, unitSetMsg.mode, unitSetMsg.unitId);
    auto bakTargetNo = targetNo;
    if (fixed)
    {
        bakTargetNo = 1;
    }

    auto& targetList = unit.multiSettingMap.value(bakTargetNo);
    for (auto& item : targetList)
    {
        auto find = src.find(item.id);
        if (find == src.end())
        {
            continue;
        }
        auto groupName = unit.getMultiParamGroupName(targetNo, fixed);  // 获取分组名
        auto key = groupName.isEmpty() ? item.desc : QString("%1:%2").arg(groupName, item.desc);

        auto paramValue = find.value();
        if (item.displayFormat == DisplayFormat::DisplayFormat_Enum)
        {
            auto enumEntryList = unit.enumMap[item.enumType].emumEntryList;
            for (auto enumEntry : enumEntryList)
            {
                if (enumEntry.uValue == paramValue)
                {
                    paramDescValueMap[key] = enumEntry.desc;
                    break;
                }
            }
        }
        else
        {
            paramDescValueMap[key] = paramValue;
        }
    }
}
bool SingleCommandHelper::createMultiPackMsg(const MessageAddress& targetAddr, const SingleCommand& singleCmd, PackCommand& packCommand,
                                             PackMessage& packMsg) const
{
    auto createErrorMsg = [&](const QString& str = QString()) {
        int intDeviceId;
        singleCmd.deviceId >> intDeviceId;
        return QString("单命令%1组包失败：%2，设备ID：0x%3，模式id：0x%4，单元id：%5")
            .arg(singleCmd.groupName, (str.isEmpty() ? "计算通道有效标识目标号越界" : str))
            .arg(intDeviceId, 0, 16)
            .arg(singleCmd.modeId, 0, 16)
            .arg(singleCmd.cmdId);
    };

    UnitParamSetMessage unitSetMsg;
    unitSetMsg.mode = singleCmd.modeId;
    unitSetMsg.unitId = singleCmd.cmdId;

    if (singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::Skuo2) ||
        singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::KaKuo2))
    {
        /* 0公共  4个目标1-4
         * 但是扩频这里公共部分和扩频部分是一体的
         * 而且扩频部分不能为空 不可能出现只更改公共部分的情况
         * 所以0 和 1 代表的是一样的 都是2
         */
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.channelValidIdent = 2;
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // 获取下发的参数名称和对应的值描述
            getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
        }
        else if (singleCmd.targetNo >= 1 && singleCmd.targetNo <= 4)
        {
            unitSetMsg.channelValidIdent = singleCmd.targetNo + 1;
            unitSetMsg.multiTargetParamMap[singleCmd.targetNo] = singleCmd.paramMap;
            // 获取下发的参数名称和对应的值描述
            getUnitParamSetCmdMultiParamDesc(targetAddr, unitSetMsg, singleCmd.paramMap, packCommand.paramDescValueMap, singleCmd.targetNo, true);
        }
        else
        {
            packCommand.errorMsg = createErrorMsg();
            return false;
        }
    }
    else if (singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHSMJ) ||
             singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHWX) ||
             singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHGML))
    {
        /* 0公共  1一体化 */
        unitSetMsg.channelValidIdent = singleCmd.targetNo;
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // 获取下发的参数名称和对应的值描述
            getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
        }
        else if (singleCmd.targetNo == 1)
        {
            unitSetMsg.multiTargetParamMap[singleCmd.targetNo] = singleCmd.paramMap;
            // 获取下发的参数名称和对应的值描述
            getUnitParamSetCmdMultiParamDesc(targetAddr, unitSetMsg, singleCmd.paramMap, packCommand.paramDescValueMap, singleCmd.targetNo, false);
        }
        else
        {
            packCommand.errorMsg = createErrorMsg();
            return false;
        }
    }

    else if (singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHSMJK2GZB) ||
             singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHSMJK2BGZB) ||
             singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHWXSK2) ||
             singleCmd.modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode::SYTHGMLSK2))
    {
        /* 0公共  1一体化 2扩频*/
        unitSetMsg.channelValidIdent = singleCmd.targetNo;
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // 获取下发的参数名称和对应的值描述
            getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
        }
        else if (singleCmd.targetNo == 1 || singleCmd.targetNo == 2)
        {
            unitSetMsg.multiTargetParamMap[singleCmd.targetNo] = singleCmd.paramMap;
            getUnitParamSetCmdMultiParamDesc(targetAddr, unitSetMsg, singleCmd.paramMap, packCommand.paramDescValueMap, singleCmd.targetNo, false);
        }
        else
        {
            packCommand.errorMsg = createErrorMsg();
            return false;
        }
    }
    else
    {
        packCommand.errorMsg = createErrorMsg("未知的模式");
        return false;
    }

    packMsg.unitParamSetMsg = unitSetMsg;
    return true;
}

bool SingleCommandHelper::createPackMsg(const MessageAddress& targetAddr, const SingleCommand& singleCmd, PackCommand& packCommand,
                                        PackMessage& packMsg) const
{
    UnitParamSetMessage unitSetMsg;
    unitSetMsg.mode = singleCmd.modeId;
    unitSetMsg.unitId = singleCmd.cmdId;
    unitSetMsg.channelValidIdent = 0;  // 暂时只支持无多目标的单元参数设置
    unitSetMsg.settingParamMap = singleCmd.paramMap;
    packMsg.unitParamSetMsg = unitSetMsg;

    // 获取下发的参数名称和对应的值描述
    getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
    return true;
}
