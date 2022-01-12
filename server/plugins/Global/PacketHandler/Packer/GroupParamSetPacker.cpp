#include "GroupParamSetPacker.h"

#include "CConverter.h"
#include "DevGeneralDefine.h"
#include "GlobalData.h"
#include "PacketHandler.h"

GroupParamSetPacker::GroupParamSetPacker(QObject* parent)
    : BasePacker(parent)
{
}

GroupParamSetPacker::~GroupParamSetPacker() {}

DevMsgType GroupParamSetPacker::messageType() const { return DevMsgType::GroupParamSetCmd; }

bool GroupParamSetPacker::packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const
{
    const auto& targetAddr = message.header.targetAddr;
    auto modeId = message.groupParamSetMsg.modeId;
    auto unitParamSetMsgMap = message.groupParamSetMsg.unitParamSetMsgMap;

    // 获取该设备该模式下的全部单元
    auto unitMap = GlobalData::getUnit(targetAddr, modeId);
    if (unitMap.isEmpty())
    {
        errorMsg = QString("获取全部单元数据失败");
        return false;
    }

    // 确保组参数包含全部单元，如果没有包含就添加状态上报的数据
    for (auto unitId : unitMap.keys())
    {
        if (!unitParamSetMsgMap.contains(unitId))
        {
            // 获取单元上报的数据
            auto unitReportMsg = GlobalData::getUnitReportData(targetAddr.systemNumb, targetAddr.deviceNumb, targetAddr.extenNumb, modeId, unitId);

            UnitParamSetMessage unitParamSetMsg;
            unitParamSetMsg.unitId = unitId;
            unitParamSetMsg.mode = modeId;
            unitParamSetMsg.channelValidIdent = 0;
            unitParamSetMsg.settingParamMap = unitReportMsg.paramMap;
            unitParamSetMsg.multiTargetParamMap = unitReportMsg.multiTargetParamMap;

            unitParamSetMsgMap[unitId] = unitParamSetMsg;
        }
    }

    // 添加顺序号
    bodyBytes.append(CConverter::toByteArray<ushort>(0));

    // 添加组参数数据
    for (auto unitId : unitMap.keys())
    {
        auto unitParamSetMsg = unitParamSetMsgMap.value(unitId);

        // 获取单元上报的数据
        auto unitReportMsg = GlobalData::getUnitReportData(targetAddr.systemNumb, targetAddr.deviceNumb, targetAddr.extenNumb, unitParamSetMsg.mode,
                                                           unitParamSetMsg.unitId);
        // 单目标设置参数值插入状态上报的值
        for (auto paramId : unitReportMsg.paramMap.keys())
        {
            if (!unitParamSetMsg.settingParamMap.contains(paramId))
            {
                unitParamSetMsg.settingParamMap.insert(paramId, unitReportMsg.paramMap[paramId]);
            }
        }

        // 多目标设置参数插入状态上报的值
        for (auto targetNo : unitReportMsg.multiTargetParamMap.keys())
        {
            // 需要下发的参数不包含该目标，直接使用上报的参数
            if (!unitParamSetMsg.multiTargetParamMap.contains(targetNo))
            {
                unitParamSetMsg.multiTargetParamMap[targetNo] = unitReportMsg.multiTargetParamMap[targetNo];
            }
            else
            {
                const auto& targetReportMap = unitReportMsg.multiTargetParamMap.value(targetNo);
                auto& targetParamMap = unitParamSetMsg.multiTargetParamMap[targetNo];

                for (auto paramId : targetReportMap.keys())
                {
                    if (!targetParamMap.contains(paramId))
                    {
                        targetParamMap.insert(paramId, targetReportMap.value(paramId));
                    }
                }
            }
        }

        qDebug() << QString("***********************************************%1***********************************************")
                        .arg(GlobalData::getUnitName(targetAddr, unitParamSetMsg.mode, unitParamSetMsg.unitId));
        QByteArray unitParamBytes;
        if (!packUnitParams(targetAddr, unitParamSetMsg, unitParamBytes))
        {
            return false;
        }
        qDebug() << unitParamBytes.toHex().toUpper();
        bodyBytes.append(0xFB);
        bodyBytes.append(CConverter::toByteArray<quint8>(unitParamSetMsg.unitId));
        bodyBytes.append(unitParamBytes);
        bodyBytes.append(0xFE);
        bodyBytes.append(CConverter::toByteArray<quint8>(unitParamSetMsg.unitId));
    }

    return true;
}

bool GroupParamSetPacker::packUnitParams(const MessageAddress& targetAddr, const UnitParamSetMessage& unitParamSetMsg, QByteArray& unitBytes) const
{
    // 获取单元参数
    Unit unit = GlobalData::getUnit(targetAddr, unitParamSetMsg.mode, unitParamSetMsg.unitId);
    if (!unit.isValid())
    {
        return false;
    }

    // 组公共设置参数
    QByteArray paramBytes;
    if (!packParams(unit.parameterSetList, unitParamSetMsg.settingParamMap, paramBytes))
    {
        return false;
    }
    unitBytes.append(paramBytes);

    // 组多目标设置参数
    for (auto targetNo : unit.multiSettingMap.keys())
    {
        auto params = unit.multiSettingMap[targetNo];
        auto unitMultiTarget = unit.targetData[targetNo];

        for (int i = targetNo; i < targetNo + unitMultiTarget.recycleNum.toInt(); ++i)
        {
            auto targetParamValueMap = unitParamSetMsg.multiTargetParamMap[i];

            QByteArray paramBytes;
            if (!packParams(params, targetParamValueMap, paramBytes))
            {
                return false;
            }

            unitBytes.append(paramBytes);
        }
    }

    return true;
}
