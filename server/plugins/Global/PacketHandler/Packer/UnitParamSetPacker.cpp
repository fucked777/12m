#include "UnitParamSetPacker.h"

#include "CConverter.h"
#include "DevGeneralDefine.h"
#include "DevProtocol.h"
#include "GlobalData.h"

UnitParamSetPacker::UnitParamSetPacker(QObject* parent)
    : BasePacker(parent)
{
}
UnitParamSetPacker::~UnitParamSetPacker() {}

DevMsgType UnitParamSetPacker::messageType() const { return DevMsgType::UnitParameterSetCmd; }

bool UnitParamSetPacker::packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const
{
    const auto& targetAddr = message.header.targetAddr;
    auto unitParamSetMsg = message.unitParamSetMsg;

    bodyBytes.resize(0);
    // 添加单元标识
    bodyBytes.append(CConverter::toByteArray<quint8>(unitParamSetMsg.unitId));

    // 添加顺序号
    bodyBytes.append(CConverter::toByteArray<ushort>(0));

    // 获取单元上报的数据
    auto unitReportMsg = GlobalData::getUnitReportData(targetAddr.systemNumb, targetAddr.deviceNumb, targetAddr.extenNumb, unitParamSetMsg.mode,
                                                       unitParamSetMsg.unitId);
    // 单目标设置参数值插入状态上报的值
    for (auto paramId : unitReportMsg.paramMap.keys())
    {
        if (!unitParamSetMsg.settingParamMap.contains(paramId))
        {
            unitParamSetMsg.settingParamMap.insert(paramId, unitReportMsg.paramMap.value(paramId));
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

    // 添加单元参数数据
    QByteArray unitBytes;
    if (!packUnitParams(targetAddr, unitParamSetMsg, unitBytes, errorMsg))
    {
        return false;
    }
    bodyBytes.append(unitBytes);

    return true;
}

bool UnitParamSetPacker::packUnitParams(const MessageAddress& targetAddr, const UnitParamSetMessage& unitParamSetMsg, QByteArray& unitBytes,
                                        QString& errorMsg) const
{
    // 获取单元参数
    Unit unit = GlobalData::getUnit(targetAddr, unitParamSetMsg.mode, unitParamSetMsg.unitId);
    if (!unit.isValid())
    {
        DeviceID deviceID(targetAddr.systemNumb, targetAddr.deviceNumb, targetAddr.extenNumb);
        int intDeviceId;
        deviceID >> intDeviceId;
        errorMsg = QString("未匹配到需要下发的单元，设备ID：0x%1，模式id：0x%2，单元id：%3")
                       .arg(QString::number(intDeviceId, 16))
                       .arg(QString::number(unitParamSetMsg.mode, 16).toUpper())
                       .arg(unitParamSetMsg.unitId);
        return false;
    }

    if (unit.multiSettingMap.isEmpty())  // 组单目标设置参数
    {
        // 跟踪基带模式  特殊处理跟踪基带一体化+扩2
        //        if (targetAddr.systemNumb == 0x4 && targetAddr.deviceNumb == 0x2 && (unitParamSetMsg.unitId == 1))
        //        {
        //            switch (unitParamSetMsg.mode)
        //            {
        //            case static_cast<qint32>(TBMode::SMCDTI_SKTTC):
        //            {
        //                return packYTHUnitParams(unitParamSetMsg, unit, unitBytes);
        //            }
        //            default:
        //            {
        //                return packStandardMultiUnitParams(unitParamSetMsg, unit, unitBytes);
        //            }
        //            }
        //        }
        //        else
        //        {
        QByteArray paramBytes;
        if (!packParams(unit.parameterSetList, unitParamSetMsg.settingParamMap, paramBytes))
        {
            return false;
        }
        unitBytes.append(paramBytes);
        //}

        return true;
    }
    else  // 组多目标设置参数
    {
        // 测控基带
        if (targetAddr.systemNumb == 0x4 && targetAddr.deviceNumb == 0x0)
        {
            switch (unitParamSetMsg.mode)
            {
            case static_cast<qint32>(MACBMode::SMCDTI_AL):
            case static_cast<qint32>(MACBMode::SMCDTI_Sate):
            case static_cast<qint32>(MACBMode::SMCDTI_UCR):
            {
                return packYTHUnitParams(unitParamSetMsg, unit, unitBytes);
            }
            case static_cast<qint32>(MACBMode::SMCDTI_ALNC):
            case static_cast<qint32>(MACBMode::SMCDTI_ALC):
            case static_cast<qint32>(MACBMode::SMCDTI_SateK2):
            case static_cast<qint32>(MACBMode::SMCDTI_UCRK2):
            {
                return packYTH_KPUnitParams(unitParamSetMsg, unit, unitBytes);
            }
            default:
            {
                return packStandardMultiUnitParams(unitParamSetMsg, unit, unitBytes);
            }
            }
        }
        // 跟踪基带模式
        else if (targetAddr.systemNumb == 0x4 && targetAddr.deviceNumb == 0x2)
        {
            switch (unitParamSetMsg.mode)
            {
                //            case static_cast<qint32>(TBMode::SMCDTI_SKTTC):
                //            {
                //                return packYTHUnitParams(unitParamSetMsg, unit, unitBytes);
                //            }
            default:
            {
                return packStandardMultiUnitParams(unitParamSetMsg, unit, unitBytes);
            }
            }
        }
        // 联试应答机
        else if (targetAddr.systemNumb == 0x6 && targetAddr.deviceNumb == 0x0)
        {
            switch (unitParamSetMsg.mode)
            {
            // case static_cast<qint32>(JTRMode::TI_AL): return packYTHUnitParams(unitParamSetMsg, unit, unitBytes);
            default:
            {
                return packStandardMultiUnitParams(unitParamSetMsg, unit, unitBytes);
            }
            }
        }
        else
        {
            return packStandardMultiUnitParams(unitParamSetMsg, unit, unitBytes);
        }
    }
}

bool UnitParamSetPacker::packStandardMultiUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const
{
    // 添加通道有效标识
    ushort channelValidIdent = 0;
    for (int i = 0; i < unitParamSetMsg.channelValidIdent; ++i)
    {
        channelValidIdent = (channelValidIdent << 1) | 0x1;
    }
    bodyBytes.append(CConverter::toByteArray<ushort>(channelValidIdent));

    // 组公共参数
    if (!unit.parameterSetList.isEmpty())
    {
        QByteArray paramBytes;
        if (!packParams(unit.parameterSetList, unitParamSetMsg.settingParamMap, paramBytes))
        {
            return false;
        }
        // 添加公共参数长度
        bodyBytes.append(CConverter::toByteArray<quint16>(paramBytes.length()));
        bodyBytes.append(paramBytes);
    }

    // 添加各个目标参数
    for (auto targetNo : unit.multiSettingMap.keys())
    {
        auto params = unit.multiSettingMap[targetNo];
        auto unitMultiTarget = unit.targetData[targetNo];

        for (int i = targetNo; i < targetNo + unitMultiTarget.recycleNum.toInt(); ++i)
        {
            auto targetParamValueMap = unitParamSetMsg.multiTargetParamMap[i];

            QByteArray paramBytes;

            //以下做参数分组使用
            QList<ParameterAttribute> commomParamValues;  //公共参数Iter
            QList<ParameterAttribute> YC_ParamValues;     //公共参数Iter
            QList<ParameterAttribute> CL_ParamValues;     //公共参数Iter
            for (auto item : params)
            {
                if (item.id.startsWith("YC_"))
                {
                    YC_ParamValues.append(item);
                }
                else if (item.id.startsWith("CL_"))
                {
                    CL_ParamValues.append(item);
                }
                else
                {
                    commomParamValues.append(item);
                }
            }

            QMap<QString, QVariant> commomParamValueMap;  //公共参数
            QMap<QString, QVariant> YC_ParamValueMap;     //遥测参数
            QMap<QString, QVariant> CL_ParamValueMap;     //测量参数

            for (auto key : targetParamValueMap.keys())
            {
                if (key.startsWith("YC_"))
                {
                    YC_ParamValueMap[key] = targetParamValueMap[key];
                }
                else if (key.startsWith("CL_"))
                {
                    CL_ParamValueMap[key] = targetParamValueMap[key];
                }
                else
                {
                    commomParamValueMap[key] = targetParamValueMap[key];
                }
            }

            QByteArray commomParamBytes;
            QByteArray YC_ParamBytes;
            QByteArray CL_ParamBytes;

            if (!commomParamValues.isEmpty())
            {
                if (!packParams(commomParamValues, commomParamValueMap, commomParamBytes))
                {
                    return false;
                }  // 添加目标参数长度
                paramBytes.append(CConverter::toByteArray<quint16>(commomParamBytes.length()));
                paramBytes.append(commomParamBytes);
            }
            if (!YC_ParamValues.isEmpty())
            {
                if (!packParams(YC_ParamValues, YC_ParamValueMap, YC_ParamBytes))
                {
                    return false;
                }
                // 添加目标参数长度
                paramBytes.append(CConverter::toByteArray<quint16>(YC_ParamBytes.length()));
                paramBytes.append(YC_ParamBytes);
            }
            if (!CL_ParamValues.isEmpty())
            {
                if (!packParams(CL_ParamValues, CL_ParamValueMap, CL_ParamBytes))
                {
                    return false;
                }
                // 添加目标参数长度
                paramBytes.append(CConverter::toByteArray<quint16>(CL_ParamBytes.length()));
                paramBytes.append(CL_ParamBytes);
            }

            bodyBytes.append(paramBytes);
        }
    }

    return true;
}

bool UnitParamSetPacker::packYTHUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const
{
    // 添加通道有效标识
    if (unitParamSetMsg.channelValidIdent < 0)
    {
        qDebug() << "通道有效标识错误1";
        return false;
    }
    quint16 channelValidIdent = 0;
    channelValidIdent = 1 << unitParamSetMsg.channelValidIdent;
    bodyBytes.append(CConverter::toByteArray<quint16>(channelValidIdent));

    // 组公共参数
    if (unitParamSetMsg.channelValidIdent == 0)
    {
        QByteArray paramBytes;
        if (!packParams(unit.parameterSetList, unitParamSetMsg.settingParamMap, paramBytes))
        {
            return false;
        }
        // 添加公共参数长度
        bodyBytes.append(CConverter::toByteArray<quint16>(paramBytes.length()));
        bodyBytes.append(paramBytes);
    }
    else
    {
        // 通过有效通道标识获取对应的目标参数
        auto paramsIter = unit.multiSettingMap.find(unitParamSetMsg.channelValidIdent);
        if (paramsIter == unit.multiSettingMap.end())
        {
            qDebug() << "通道有效标识错误2";
            return false;
        }

        // 只组有效的目标参数
        auto targetParamValueMap = unitParamSetMsg.multiTargetParamMap[unitParamSetMsg.channelValidIdent];
        QByteArray paramBytes;
        if (!packParams(paramsIter.value(), targetParamValueMap, paramBytes))
        {
            return false;
        }
        // 添加目标参数长度
        bodyBytes.append(CConverter::toByteArray<quint16>(paramBytes.length()));
        bodyBytes.append(paramBytes);
    }

    return true;
}

bool UnitParamSetPacker::packYTH_KPUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const
{
    // 添加通道有效标识
    if (unitParamSetMsg.channelValidIdent < 0)
    {
        qDebug() << "通道有效标识错误1";
        return false;
    }
    quint16 channelValidIdent = 0;
    channelValidIdent = 1 << unitParamSetMsg.channelValidIdent;
    bodyBytes.append(CConverter::toByteArray<quint16>(channelValidIdent));

    // 组公共参数
    if (unitParamSetMsg.channelValidIdent == 0)
    {
        QByteArray paramBytes;
        if (!packParams(unit.parameterSetList, unitParamSetMsg.settingParamMap, paramBytes))
        {
            return false;
        }
        // 添加公共参数长度
        bodyBytes.append(CConverter::toByteArray<quint16>(paramBytes.length()));
        bodyBytes.append(paramBytes);
    }
    else
    {
        // 通过有效通道标识获取对应的目标参数
        auto paramsIter = unit.multiSettingMap.find(unitParamSetMsg.channelValidIdent);
        if (paramsIter == unit.multiSettingMap.end())
        {
            qDebug() << "通道有效标识错误2";
            return false;
        }

        // 只组有效的目标参数
        auto targetParamValueMap = unitParamSetMsg.multiTargetParamMap[unitParamSetMsg.channelValidIdent];

        QByteArray paramBytes;

        //以下做参数分组使用
        QList<ParameterAttribute> commomParamValues;  //公共参数Iter
        QList<ParameterAttribute> YC_ParamValues;     //公共参数Iter
        QList<ParameterAttribute> CL_ParamValues;     //公共参数Iter
        for (auto item : paramsIter.value())
        {
            if (item.id.startsWith("YC_"))
            {
                YC_ParamValues.append(item);
            }
            else if (item.id.startsWith("CL_"))
            {
                CL_ParamValues.append(item);
            }
            else
            {
                commomParamValues.append(item);
            }
        }

        QMap<QString, QVariant> commomParamValueMap;  //公共参数
        QMap<QString, QVariant> YC_ParamValueMap;     //遥测参数
        QMap<QString, QVariant> CL_ParamValueMap;     //测量参数

        for (auto key : targetParamValueMap.keys())
        {
            if (key.startsWith("YC_"))
            {
                YC_ParamValueMap[key] = targetParamValueMap[key];
            }
            else if (key.startsWith("CL_"))
            {
                CL_ParamValueMap[key] = targetParamValueMap[key];
            }
            else
            {
                commomParamValueMap[key] = targetParamValueMap[key];
            }
        }

        QByteArray commomParamBytes;
        QByteArray YC_ParamBytes;
        QByteArray CL_ParamBytes;

        if (!commomParamValues.isEmpty())
        {
            if (!packParams(commomParamValues, commomParamValueMap, commomParamBytes))
            {
                return false;
            }
            // 添加目标参数长度
            paramBytes.append(CConverter::toByteArray<quint16>(commomParamBytes.length()));
            paramBytes.append(commomParamBytes);
        }
        if (!YC_ParamValues.isEmpty())
        {
            if (!packParams(YC_ParamValues, YC_ParamValueMap, YC_ParamBytes))
            {
                return false;
            }
            // 添加目标参数长度
            paramBytes.append(CConverter::toByteArray<quint16>(YC_ParamBytes.length()));
            paramBytes.append(YC_ParamBytes);
        }
        if (!CL_ParamValues.isEmpty())
        {
            if (!packParams(CL_ParamValues, CL_ParamValueMap, CL_ParamBytes))
            {
                return false;
            }
            // 添加目标参数长度
            paramBytes.append(CConverter::toByteArray<quint16>(CL_ParamBytes.length()));
            paramBytes.append(CL_ParamBytes);
        }

        bodyBytes.append(paramBytes);
    }

    return true;
}
