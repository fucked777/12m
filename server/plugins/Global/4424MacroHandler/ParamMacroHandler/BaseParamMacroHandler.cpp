#include "BaseParamMacroHandler.h"

#include "DevProtocol.h"
#include "GlobalData.h"
#include "GroupParamSetPacker.h"
#include "LocalCommunicationAddr.h"
#include "PacketHandler.h"
#include "ParamMacroMessageSerialize.h"
#include "SatelliteManagementDefine.h"
#include "UnitParamSetPacker.h"

BaseParamMacroHandler::BaseParamMacroHandler(QObject* parent)
    : BaseHandler(parent)
{
    PacketHandler::instance().appendPacker(new GroupParamSetPacker());
    PacketHandler::instance().appendPacker(new UnitParamSetPacker());
}

bool BaseParamMacroHandler::packGroupParamSetData(const DeviceID& deviceID, int modeId,
                                                  const QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap,
                                                  PackCommand& packCommand) const
{
    //  重置命令信息
    packCommand.reset();

    // 转为 PackCommand
    packCommand.deviceID = deviceID;
    packCommand.modeId = modeId;
    packCommand.cmdType = DevMsgType::GroupParamSetCmd;
    packCommand.operatorInfo = QString("下发%1参数宏").arg(GlobalData::getExtensionName(deviceID));

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = deviceID.sysID;
    targetAddr.deviceNumb = deviceID.devID;
    targetAddr.extenNumb = deviceID.extenID;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;

    GroupParamSetMessage groupParamCmdMsg;
    groupParamCmdMsg.modeId = modeId;

    for (auto unitId : unitTargetParamMap.keys())
    {
        UnitParamSetMessage unitMsg;
        unitMsg.unitId = unitId;
        unitMsg.mode = modeId;

        auto unitParamMap = unitTargetParamMap.value(unitId);
        for (auto targetNo : unitParamMap.keys())
        {
            auto targetParamMap = unitParamMap.value(targetNo);
            if (targetNo == 0)  // 公共参数
            {
                unitMsg.settingParamMap = targetParamMap;
            }
            else
            {
                unitMsg.multiTargetParamMap[targetNo] = targetParamMap;
            }
        }

        groupParamCmdMsg.unitParamSetMsgMap[unitId] = unitMsg;
    }

    PackMessage packMsg;
    packMsg.header.msgType = DevMsgType::GroupParamSetCmd;
    packMsg.header.sourceAddr = sourceAddr;
    packMsg.header.targetAddr = targetAddr;
    packMsg.header.version = version;
    packMsg.groupParamSetMsg = groupParamCmdMsg;

    QString erroMsg;
    return PacketHandler::instance().pack(packMsg, packCommand.data, erroMsg);
}

bool BaseParamMacroHandler::getPointFreqParamMap(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, const DeviceID& deviceID,
                                                 QMap<int, QMap<QString, QVariant>>& unitParamMap)
{
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);

    // 获取指定任务代号指定模式的参数宏数据
    ParamMacroModeData paramMacroModeData;
    if (!GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeData))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(taskCode).arg(workModeDesc));
        return false;
    }

    // 获取点频下的单元参数
    if (!paramMacroModeData.getPointFreqUnitParamMap(pointFreqNo, deviceID, unitParamMap))
    {
        emit signalErrorMsg(QString("任务代号为%1的参数宏，模式为%2，点频为%3的%4参数失败")
                                .arg(taskCode, workModeDesc)
                                .arg(pointFreqNo)
                                .arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }

    // 获取设备参数
    QMap<int, QMap<QString, QVariant>> deviceUnitParamMap;
    if (!paramMacroModeData.getDeviceUnitParamMap(deviceID, deviceUnitParamMap))
    {
        emit signalErrorMsg(
            QString("获取任务代号为%1的参数宏的%2模式，%3设备参数失败").arg(taskCode).arg(workModeDesc).arg(GlobalData::getExtensionName(deviceID)));
        return false;
    }
    for (auto unitId : deviceUnitParamMap.keys())
    {
        for (auto paramId : deviceUnitParamMap.value(unitId).keys())
        {
            unitParamMap[unitId][paramId] = deviceUnitParamMap[unitId][paramId];
        }
    }

    // QMap<动态参数组id, QMap<单元id, QMap<第几组参数, QMap<参数id, 参数值>>>>
    QMap<QString, QMap<int, QMap<int, QMap<QString, QVariant>>>> dynamicParamMap;
    paramMacroModeData.getDynamicParamMap(pointFreqNo, deviceID, dynamicParamMap);

    // 获取卫星
    SatelliteManagementData satelliteData;
    if (!GlobalData::getSatelliteManagementData(taskCode, satelliteData))
    {
        emit signalErrorMsg(QString("卫星代号：%1无法找到").arg(taskCode));
        return false;
    }
    // 从卫星管理获取选择的第几组动态参数
    for (auto dynamicGroupId : dynamicParamMap.keys())
    {
        int groupNo = 0;
        satelliteData.getSelectedGroupNo(workMode, pointFreqNo, groupNo, dynamicGroupId);

        auto unitDynamicParaMap = dynamicParamMap.value(dynamicGroupId);
        for (auto unitId : unitDynamicParaMap.keys())
        {
            auto dynamicParamGroupMap = unitDynamicParaMap.value(unitId);
            auto paramMap = dynamicParamGroupMap.value(groupNo);
            for (auto paramId : paramMap.keys())
            {
                unitParamMap[unitId][paramId] = paramMap.value(paramId);
            }
        }
    }

    return true;
}

bool BaseParamMacroHandler::getUnitTargetParamMap(const DeviceID& deviceID, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);

    QMap<int, QMap<QString, QVariant>> targetTaskIdAndCodeMap;

    // 目标单元参数 QMap<第几个目标, QMap<单元id, QMap<参数id, 值>>>
    QMap<int, QMap<int, QMap<QString, QVariant>>> targetUnitParamMap;
    for (auto targetInfo : mLinkLine.targetMap)
    {
        auto taskCode = targetInfo.taskCode;
        auto pointFreqNo = targetInfo.pointFreqNo;

        // 获取目标的工作模式
        auto targetWorkMode = SystemWorkModeHelper::getTargetWorkMode(mLinkLine.workMode, targetInfo.targetNo);

        // 获取指定任务代号指定模式的参数宏数据
        ParamMacroModeData paramMacroModeData;
        if (!GlobalData::getParamMacroModeData(taskCode, targetWorkMode, paramMacroModeData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(taskCode).arg(workModeDesc));
            return false;
        }

        // 获取点频下的设备单元参数
        QMap<int, QMap<QString, QVariant>> unitParamMap;
        if (!paramMacroModeData.getPointFreqUnitParamMap(pointFreqNo, deviceID, unitParamMap))
        {
            emit signalErrorMsg(QString("任务代号为%1的参数宏，模式为%2，点频为%3的%4参数失败")
                                    .arg(taskCode, workModeDesc)
                                    .arg(pointFreqNo)
                                    .arg(GlobalData::getExtensionName(deviceID)));
            return false;
        }

        // 获取卫星
        SatelliteManagementData satelliteData;
        if (!GlobalData::getSatelliteManagementData(taskCode, satelliteData))
        {
            emit signalErrorMsg(QString("卫星代号：%1无法找到").arg(taskCode));
            return false;
        }
        // 记录卫星代号和标识
        targetTaskIdAndCodeMap[targetInfo.targetNo]["TaskIdent"] = satelliteData.m_satelliteIdentification;
        targetTaskIdAndCodeMap[targetInfo.targetNo]["TaskCode"] = satelliteData.m_satelliteCode;

        // QMap<动态参数组id, QMap<单元id, QMap<第几组参数, QMap<参数id, 参数值>>>>
        QMap<QString, QMap<int, QMap<int, QMap<QString, QVariant>>>> dynamicParamMap;
        paramMacroModeData.getDynamicParamMap(pointFreqNo, deviceID, dynamicParamMap);

        // 从卫星管理获取选择的第几组动态参数
        for (auto dynamicGroupId : dynamicParamMap.keys())
        {
            int groupNo = 0;
            satelliteData.getSelectedGroupNo(targetWorkMode, pointFreqNo, groupNo, dynamicGroupId);

            auto unitDynamicParaMap = dynamicParamMap.value(dynamicGroupId);
            for (auto unitId : unitDynamicParaMap.keys())
            {
                auto dynamicParamGroupMap = unitDynamicParaMap.value(unitId);
                auto paramMap = dynamicParamGroupMap.value(groupNo);
                for (auto paramId : paramMap.keys())
                {
                    unitParamMap[unitId][paramId] = paramMap.value(paramId);
                }
            }
        }

        targetUnitParamMap[targetInfo.targetNo] = unitParamMap;
    }

    // 把参数宏中配置的目标点频参数根据单元合并到一起
    QMap<int, QMap<int, QMap<QString, QVariant>>> paramMacroTempUnitTargetParamMap;
    for (auto targetNo : targetUnitParamMap.keys())
    {
        auto unitParamMap = targetUnitParamMap.value(targetNo);
        for (auto unitId : unitParamMap.keys())
        {
            auto paramMap = unitParamMap.value(unitId);
            for (auto paramId : paramMap.keys())
            {
                paramMacroTempUnitTargetParamMap[unitId][targetNo][paramId] = paramMap.value(paramId);
            }
        }
    }

    // 获取该链路的模式id
    auto linkLineModeId = SystemWorkModeHelper::systemWorkModeConverToModeId(mLinkLine.workMode);
    // 获取链路模式的模式参数
    auto mode = GlobalData::getMode(deviceID, linkLineModeId);

    // 根据设备监控的配置设备协议参数，如果参数宏配置了该单元的参数就放到一起
    for (auto unitId : mode.unitMap.keys())  // 循环单元
    {
        auto unit = mode.unitMap.value(unitId);
        auto paramMacroTargetParamMap = paramMacroTempUnitTargetParamMap.value(unitId);

        for (auto param : unit.parameterSetList)  // 公共参数
        {
            for (auto targetNo : paramMacroTargetParamMap.keys())
            {
                auto targetParamMap = paramMacroTargetParamMap.value(targetNo);
                if ((targetNo == mLinkLine.masterTargetNo) && targetParamMap.contains(param.id))  // 公共参数设置为主跟目标的
                {
                    unitTargetParamMap[unitId][0][param.id] = targetParamMap[param.id];
                }
            }
        }

        for (auto multiTargetNo : unit.multiSettingMap.keys())  // 循环单元的多目标参数
        {
            auto recycleNum = unit.targetData.value(multiTargetNo).recycleNum.toInt();
            auto multiSetParamList = unit.multiSettingMap.value(multiTargetNo);

            for (auto param : multiSetParamList)
            {
                for (auto targetNo : paramMacroTargetParamMap.keys())
                {
                    auto targetParamMap = paramMacroTargetParamMap.value(targetNo);
                    if (targetParamMap.contains(param.id))
                    {
                        if (recycleNum > 1)  // 主要是区分扩频目标循环4次，一体化的目标只循环1次
                        {
                            unitTargetParamMap[unitId][targetNo][param.id] = targetParamMap[param.id];
                        }
                        else
                        {
                            unitTargetParamMap[unitId][multiTargetNo][param.id] = targetParamMap[param.id];
                        }
                    }
                }
            }
        }
    }

    // 添加任务标识和任务代号
    auto firstUnit = mode.unitMap[1];
    for (auto param : firstUnit.parameterSetList)
    {
        auto taskParamMap = targetTaskIdAndCodeMap[1];
        for (auto taskParamId : taskParamMap.keys())
        {
            if (param.id.contains(taskParamId))  // 通过包含TaskIdent或者TaskCode判断
            {
                unitTargetParamMap[1][0][param.id] = taskParamMap[taskParamId];
            }
        }
    }
    for (auto multiTargetNo : firstUnit.multiSettingMap.keys())
    {
        auto recycleNum = firstUnit.targetData.value(multiTargetNo).recycleNum.toInt();
        auto multiSetParamList = firstUnit.multiSettingMap.value(multiTargetNo);

        QMap<QString, QVariant> taskParamMap;
        if (recycleNum > 1)  // 主要是区分扩频目标循环4次，一体化的目标只循环1次
        {
            for (int targetNo = 1; targetNo <= recycleNum; ++targetNo)
            {
                taskParamMap = targetTaskIdAndCodeMap.value(targetNo);
                for (auto param : multiSetParamList)
                {
                    for (auto taskParamId : taskParamMap.keys())
                    {
                        if (param.id.contains(taskParamId))  // 通过包含TaskIdent或者TaskCode判断
                        {
                            unitTargetParamMap[1][targetNo][param.id] = taskParamMap[taskParamId];
                        }
                    }
                }
            }
        }
        else
        {
            taskParamMap = targetTaskIdAndCodeMap.value(multiTargetNo);
            for (auto param : multiSetParamList)
            {
                for (auto taskParamId : taskParamMap.keys())
                {
                    if (param.id.contains(taskParamId))
                    {
                        unitTargetParamMap[1][multiTargetNo][param.id] = taskParamMap[taskParamId];
                    }
                }
            }
        }
    }

    // 如果是多目标的模式，设置目标数(不是多目标组包会自动去除，不参与组包)
    unitTargetParamMap[1][0]["TargetNum"] = targetUnitParamMap.keys().size();
    // 如果有跟踪目标，设置主用的为主跟目标
    unitTargetParamMap[1][0]["TrackTarget"] = mLinkLine.masterTargetNo;

    return true;
}

void BaseParamMacroHandler::appendDeviceParam(const QMap<int, QMap<QString, QVariant>>& deviceUnitParamMap,
                                              QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    for (auto unitId : deviceUnitParamMap.keys())
    {
        auto paramMap = deviceUnitParamMap[unitId];  // 设备参数中的单元参数

        if (!unitTargetParamMap.contains(unitId))
        {
            unitTargetParamMap[unitId] = QMap<int, QMap<QString, QVariant>>();
        }

        auto& targetParamMap = unitTargetParamMap[unitId];
        // 目标参数，目标号为0的表示公共参数，所有的点频都默认添加4个目标
        for (int i = 0; i <= 4; ++i)
        {
            if (!targetParamMap.contains(i))
            {
                targetParamMap[i] = QMap<QString, QVariant>();
            }
        }

        for (auto targetNo : targetParamMap.keys())
        {
            for (auto paramId : paramMap.keys())
            {
                targetParamMap[targetNo][paramId] = paramMap[paramId];  // 每一个目标都添加该设备参数(多于的参数不会参与组包)
            }
        }
    }
}

/***************************************************************************************************************/
bool BaseParamMacroHandler::getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode, ParamMacroModeData& paramMacroModeItem)
{
    if (mManualMsg.isExternalParamMacro)
    {
        if (!mManualMsg.paramMacroData.modeParamMap.contains(workMode))
        {
            return false;
        }
        paramMacroModeItem = mManualMsg.paramMacroData.modeParamMap.value(workMode);
    }
    else
    {
        if (!GlobalData::getParamMacroModeData(taskCode, workMode, paramMacroModeItem))
        {
            return false;
        }
    }
    return true;
}
