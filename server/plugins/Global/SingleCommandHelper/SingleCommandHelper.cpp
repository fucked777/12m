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
    //  ??????????????????
    packCommand.reset();

    /* 20210830 wp?? ????????????????????? */
    auto singleCmdIter = mSingleCmdMap.find(singleCmdId.toUpper());
    if (singleCmdIter == mSingleCmdMap.end())
    {
        packCommand.errorMsg = QString("???????????????????????????Id???%1????????????").arg(singleCmdId);
        return false;
    }

    // ????????????????????????????????????
    auto singleCmd = singleCmdIter.value();

    // ?????????????????????Id???????????????????????????????????????ID???????????????
    if (replaceDeviceID.isValid())
    {
        singleCmd.deviceId = replaceDeviceID;
    }

    // ?????????????????????????????????id
    if (replaceModeId != -1)
    {
        singleCmd.modeId = replaceModeId;
    }

    // ?????????????????????????????????????????????????????????
    if (!replaceParamMap.isEmpty())
    {
        for (auto paramId : replaceParamMap.keys())
        {
            singleCmd.paramMap[paramId] = replaceParamMap[paramId];
        }
    }

    // ???????????????
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
    //  ??????????????????
    packCommand.reset();

    /* 20210830 wp?? ????????????????????? */
    auto singleCmdIter = mSingleCmdMap.find(singleCmdId.toUpper());
    if (singleCmdIter == mSingleCmdMap.end())
    {
        packCommand.errorMsg = QString("???????????????????????????Id???%1????????????").arg(singleCmdId);
        return false;
    }

    // ????????????????????????????????????
    auto singleCmd = singleCmdIter.value();

    // ?????????????????????Id???????????????????????????????????????ID???????????????
    if (replaceDeviceID.isValid())
    {
        singleCmd.deviceId = replaceDeviceID;
    }

    // ?????????????????????????????????????????????????????????
    if (!replaceParamMap.isEmpty())
    {
        for (auto paramId : replaceParamMap.keys())
        {
            singleCmd.paramMap[paramId] = replaceParamMap[paramId];
        }
    }
    singleCmd.multiParamMap = multiParamMap;

    // ???????????????
    return packSingleCommand(singleCmd, packCommand);
}

bool SingleCommandHelper::packSingleCommand(const SingleCommand& singleCmd, PackCommand& packCommand) const
{
    // ???SingleCommand ?????? PackCommand
    packCommand.deviceID = singleCmd.deviceId;
    packCommand.modeId = singleCmd.modeId;
    packCommand.cmdId = singleCmd.cmdId;
    packCommand.cmdType = singleCmd.cmdType;
    packCommand.operatorInfo = singleCmd.deviceInfo;

    // ??????
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

    // ??????????????????
    if (singleCmd.cmdType == DevMsgType::ProcessControlCmd)
    {
        ProcessControlCmdMessage processCtrlCmdMsg;
        processCtrlCmdMsg.mode = singleCmd.modeId;
        processCtrlCmdMsg.cmdId = singleCmd.cmdId;
        processCtrlCmdMsg.settingParamMap = singleCmd.paramMap;
        processCtrlCmdMsg.multiParamMap = singleCmd.multiParamMap;
        packMsg.processCtrlCmdMsg = processCtrlCmdMsg;

        // ????????????????????????????????????????????????
        getProcessControlCmdSettingParamDesc(targetAddr, processCtrlCmdMsg, packCommand.paramDescValueMap);
    }
    // ????????????????????????
    else if (singleCmd.cmdType == DevMsgType::UnitParameterSetCmd)
    {
        UnitParamSetMessage unitSetMsg;
        /* ????????????????????????????????? */
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
        packCommand.errorMsg = QString("???????????????????????????,?????????????????????");
        return false;
    }

    QString errorMsg;
    if (!PacketHandler::instance().pack(packMsg, packCommand.data, errorMsg))
    {
        packCommand.errorMsg = QString("?????????%1???????????????%2").arg(singleCmd.groupName).arg(errorMsg);
        return false;
    }

    return true;
}

void SingleCommandHelper::getProcessControlCmdSettingParamDesc(const MessageAddress& msgAddr, const ProcessControlCmdMessage& processCtrlCmdMsg,
                                                               QMap<QString, QVariant>& paramDescValueMap) const
{
    // ??????????????????
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
    // ??????????????????
    Unit unit = GlobalData::getUnit(msgAddr, unitSetMsg.mode, unitSetMsg.unitId);
    for (auto paramAttr : unit.parameterSetList)
    {
        auto paramValIter = unitSetMsg.settingParamMap.find(paramAttr.id);
        if (paramValIter != unitSetMsg.settingParamMap.end())
        {
            auto groupName = unit.getParamGroupName(paramAttr.id);  // ???????????????
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
    // ??????????????????
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
        auto groupName = unit.getMultiParamGroupName(targetNo, fixed);  // ???????????????
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
        return QString("?????????%1???????????????%2?????????ID???0x%3?????????id???0x%4?????????id???%5")
            .arg(singleCmd.groupName, (str.isEmpty() ? "???????????????????????????????????????" : str))
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
        /* 0??????  4?????????1-4
         * ?????????????????????????????????????????????????????????
         * ?????????????????????????????? ?????????????????????????????????????????????
         * ??????0 ??? 1 ????????????????????? ??????2
         */
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.channelValidIdent = 2;
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // ????????????????????????????????????????????????
            getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
        }
        else if (singleCmd.targetNo >= 1 && singleCmd.targetNo <= 4)
        {
            unitSetMsg.channelValidIdent = singleCmd.targetNo + 1;
            unitSetMsg.multiTargetParamMap[singleCmd.targetNo] = singleCmd.paramMap;
            // ????????????????????????????????????????????????
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
        /* 0??????  1????????? */
        unitSetMsg.channelValidIdent = singleCmd.targetNo;
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // ????????????????????????????????????????????????
            getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
        }
        else if (singleCmd.targetNo == 1)
        {
            unitSetMsg.multiTargetParamMap[singleCmd.targetNo] = singleCmd.paramMap;
            // ????????????????????????????????????????????????
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
        /* 0??????  1????????? 2??????*/
        unitSetMsg.channelValidIdent = singleCmd.targetNo;
        if (singleCmd.targetNo == 0)
        {
            unitSetMsg.settingParamMap = singleCmd.paramMap;
            // ????????????????????????????????????????????????
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
        packCommand.errorMsg = createErrorMsg("???????????????");
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
    unitSetMsg.channelValidIdent = 0;  // ????????????????????????????????????????????????
    unitSetMsg.settingParamMap = singleCmd.paramMap;
    packMsg.unitParamSetMsg = unitSetMsg;

    // ????????????????????????????????????????????????
    getUnitParamSetCmdParamDesc(targetAddr, unitSetMsg, packCommand.paramDescValueMap);
    return true;
}
