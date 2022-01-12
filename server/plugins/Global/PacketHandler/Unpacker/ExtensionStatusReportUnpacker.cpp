#include "ExtensionStatusReportUnpacker.h"

#include "CConverter.h"
#include "GlobalData.h"

ExtensionStatusReportUnpacker::ExtensionStatusReportUnpacker(QObject* parent)
    : BaseUnpacker(parent)
{
}

ExtensionStatusReportUnpacker::~ExtensionStatusReportUnpacker() {}

DevMsgType ExtensionStatusReportUnpacker::messageType() const { return DevMsgType::ExtensionStatusReport; }

bool ExtensionStatusReportUnpacker::unpackBody(const QByteArray& bodyBytes, UnpackMessage& message)
{
    const auto& sourceAddr = message.header.sourceAddr;
    auto& extenStatusReportMsg = message.extenStatusReportMsg;
    // 获取分机配置信息
    Extension extension = GlobalData::getExtension(sourceAddr);
    if (extension.ID == -1)
    {
        return false;
    }

    // 获取分机上报模式ID
    quint64 modeId = -1;
    if (extension.haveMode)  // 如果该分机有模式进行解模式
    {
        int idx = bodyBytes.indexOf(QByteArray::fromHex("FB01"));
        if (!unpackExtentionModeId(bodyBytes.mid(idx + 2), extension.modeAddr - 1, extension.modeLength, modeId))
        {
            return false;
        }
    }
    else
    {
        modeId = gNoModeType;
    }
    extenStatusReportMsg.modeId = modeId;

    // 获取该模式配置信息
    auto mode = extension.modeCtrlMap[modeId];
    if (mode.ID == -1)
    {
        return false;
    }

    // 解析顺序号
    // int orderNum = CConverter::fromByteArray<ushort>(bodyBytes.mid(0, 2));

    // 4424特殊解析伪码服务器代码
    if (sourceAddr.equipmentNumb == 0x24 && sourceAddr.systemNumb == 4 && sourceAddr.deviceNumb == 4)
    {
        return unpackWmBody(bodyBytes.mid(2), mode, extenStatusReportMsg);
    }
    // 4426特殊解析伪码服务器代码
    else if (sourceAddr.equipmentNumb == 0x26 && sourceAddr.systemNumb == 4 && sourceAddr.deviceNumb == 5)
    {
        return unpackWmBody(bodyBytes.mid(2), mode, extenStatusReportMsg);
    }
    // 特殊处理DTE状态（DTE没按协议做，无单元开始结束标识）
    else if (sourceAddr.systemNumb == 8 && sourceAddr.deviceNumb == 0)
    {
        return unpackDTEBody(bodyBytes.mid(2), mode, extenStatusReportMsg);
    }
    // 特殊处理储存转发
    else if (sourceAddr.systemNumb == 8 && sourceAddr.deviceNumb == 1)
    {
        return unpackSAFBody(bodyBytes.mid(2), mode, extenStatusReportMsg);
    }

    auto allUnitBytes = bodyBytes.mid(2);
    auto allUnitBytesLen = allUnitBytes.length();

    // 获取每个单元的字节数据
    QMap<int, QByteArray> unitBytesMap;
    int currentIdx = 0;
    while (currentIdx < allUnitBytesLen)
    {
        if (allUnitBytes.at(currentIdx) == char(0xFB))
        {
            // 获取单元标识
            int currentUnitId = (uchar)allUnitBytes.at(currentIdx + 1);
            auto currentUnitIter = mode.unitMap.find(currentUnitId);
            if (currentUnitIter != mode.unitMap.end())
            {
                int currentUnitLen = currentUnitIter->unitLength();
                // 获取单元结束标识位置
                int currentUnitEndIdentIdx = currentIdx + 1 + currentUnitLen + 1;
                if (currentUnitEndIdentIdx > allUnitBytes.size())
                {
                    qWarning() << GlobalData::getUnitName(sourceAddr, modeId, currentUnitId, "-")
                               << "配置文件单元参数长度和上报长度不一致，上报字节长度小于配置文件"
                               << "协议配置长度 ： " << currentUnitLen;

                    return false;
                }
                // 根据配置文件单元长度定位到单元结束标识
                if (allUnitBytes.at(currentUnitEndIdentIdx) == char(0xFE) && allUnitBytes.at(currentUnitEndIdentIdx + 1) == char(currentUnitId))
                {
                    unitBytesMap.insert(currentUnitId, allUnitBytes.mid(currentIdx + 2, currentUnitLen));
                    currentIdx = currentUnitEndIdentIdx + 2;
                    continue;
                }
                else
                {
                    qWarning() << GlobalData::getUnitName(sourceAddr, modeId, currentUnitId, "-") << "配置文件单元参数长度和上报长度不一致"
                               << "协议配置长度 ： " << currentUnitLen;
                    return false;
                }
            }
        }
        ++currentIdx;
    }

    // 解析单元数据
    for (const auto& unit : mode.unitMap)
    {
        int index = 0;
        const auto& unitBytes = unitBytesMap.value(unit.unitCode);

        // 解析单元单目标参数
        UnitReportMessage unitReportMsg;
        unitReportMsg.id = unit.unitCode;
        if (!unpackParams(unit.queryResultList, unitBytes, unitReportMsg.paramMap, index))
        {
            return false;
        }

        // 下面只是显示中文，方便调试
        //        for (auto param : unit.queryResultList)
        //        {
        //            unitReportMsg.paramMap.insert(param.desc, unitReportMsg.paramMap[param.id]);
        //        }

        // 解析单元多目标参数
        for (auto targetNo : unit.queryRecycleMap.keys())
        {
            auto params = unit.queryRecycleMap.value(targetNo);

            auto unitMultiTarget = unit.targetData.value(targetNo);
            for (int i = 0; i < unitMultiTarget.recycleNum.toInt(); ++i)  // 根据循环次数解析
            {
                QMap<QString, QVariant> paramVals;
                if (!unpackParams(params, unitBytes, paramVals, index))
                {
                    return false;
                }
                unitReportMsg.multiTargetParamMap.insert(targetNo + i, paramVals);

                // 下面只是显示中文，方便调试
                //                QMap<QString, QVariant> tempParamVals;
                //                for (auto param : params)
                //                {
                //                    tempParamVals[param.desc] = paramVals[param.id];
                //                }
                //                unitReportMsg.multiTargetParamMap.insert(targetNo + i + 10, tempParamVals);
            }
        }

        extenStatusReportMsg.unitReportMsgMap.insert(unit.unitCode, unitReportMsg);
    }

    return true;
}

bool ExtensionStatusReportUnpacker::unpackExtentionModeId(const QByteArray& unitBytes, int modeIdx, int modeLength, quint64& modeId)
{
    if (unitBytes.length() < modeIdx + modeLength)
    {
        return false;
    }
    modeId = CConverter::fromByteArray<quint64>(unitBytes.mid(modeIdx, modeLength));

    return true;
}

bool ExtensionStatusReportUnpacker::unpackWmBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode,
                                                 ExtensionStatusReportMessage& extenStatusReportMsg)
{
    // 获取每个单元的字节数据
    QMap<int, QByteArray> unitBytesMap;
    unitBytesMap[1] = allUnitParamBytes;

    // 解析单元数据
    for (const auto& unit : mode.unitMap)
    {
        int index = 0;
        auto unitBytes = unitBytesMap.value(unit.unitCode);

        // 解析单元单目标参数
        UnitReportMessage unitReportMsg;
        unitReportMsg.id = unit.unitCode;
        if (!unpackParams(unit.queryResultList, unitBytes, unitReportMsg.paramMap, index))
        {
            qWarning() << "伪码服务器解析错误点1：";
            return false;
        }

        //  伪码通道循环几次
        int recycleNumber = 0;
        for (auto param : unit.queryResultList)
        {
            if (param.id == "TargetNum")
            {
                recycleNumber = unitReportMsg.paramMap[param.id].toInt();
            }
        }

        // 解析单元多目标参数
        for (auto targetNo : unit.queryRecycleMap.keys())
        {
            auto params = unit.queryRecycleMap.value(targetNo);
            for (int i = 0; i <= recycleNumber - 1; ++i)  // 根据循环次数解析
            {
                QMap<QString, QVariant> paramVals;
                if (!unpackParams(params, unitBytes, paramVals, index))
                {
                    qWarning() << "伪码服务器解析错误点2：";
                    return false;
                }
                unitReportMsg.multiTargetParamMap.insert(targetNo + i, paramVals);
            }
        }

        extenStatusReportMsg.unitReportMsgMap.insert(unit.unitCode, unitReportMsg);
    }

    return true;
}

bool ExtensionStatusReportUnpacker::unpackDTEBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode,
                                                  ExtensionStatusReportMessage& extenStatusReportMsg)
{
    auto unit = mode.unitMap.first();  // 只有一个单元

    UnitReportMessage unitReportMsg;
    unitReportMsg.id = 1;

    int index = 0;
    for (auto param : unit.queryResultList)
    {
        QVariant paramVal;
        if (!unpackParam(param, allUnitParamBytes, paramVal, index))
        {
            return false;
        }
        unitReportMsg.paramMap[param.id] = paramVal;

        int targetNo;
        QList<ParameterAttribute> targetParams;
        unit.getRecycleTargetParams(param.id, targetNo, targetParams);  // 获取循环上报参数
        if (targetParams.isEmpty())
        {
            continue;
        }

        QList<QMap<QString, QVariant>> targetRecycleParamList;
        if (paramVal.toInt() > 50)  // 有时候设备报的值很大，导致内存增长
        {
            return false;
        }
        for (int i = 0; i < paramVal.toInt(); ++i)  // 循环上报值，表示需要循环多少次
        {
            QMap<QString, QVariant> paramMap;
            QVariant taskBZ;
            for (const auto& targetParam : targetParams)
            {
                QVariant paramVal;
                if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                {
                    return false;
                }
                paramMap[targetParam.id] = paramVal;

                if (targetParam.id == "MissionID")
                {
                    taskBZ = paramVal;
                }

                int targetNo2;
                QList<ParameterAttribute> targetParams;
                unit.getRecycleTargetParams(targetParam.id, targetNo2, targetParams);
                if (targetParams.isEmpty())
                {
                    continue;
                }

                if (paramVal.toInt() > 50)  // 有时候设备报的值很大，导致内存增长
                {
                    return false;
                }
                for (int i = 0; i < paramVal.toInt(); ++i)
                {
                    QMap<QString, QVariant> paramMap;
                    for (const auto& targetParam : targetParams)
                    {
                        QVariant paramVal;
                        if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                        {
                            return false;
                        }
                        paramMap[targetParam.id] = paramVal;
                        if (targetParam.id == "Mission1Center1")
                        {
                            unitReportMsg.dteTaskBZAndCenterBZMap[taskBZ].append(paramVal);
                        }
                    }
                    unitReportMsg.multiTargetParamMap3[targetNo][targetNo2].append(paramMap);  //这里必须用append才能获取到所有的第三种循环的数据
                }
            }
            targetRecycleParamList << paramMap;
        }
        unitReportMsg.multiTargetParamMap2[targetNo] = targetRecycleParamList;
    }

    extenStatusReportMsg.unitReportMsgMap[1] = unitReportMsg;
    return true;
}

bool ExtensionStatusReportUnpacker::unpackSAFBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode,
                                                  ExtensionStatusReportMessage& extenStatusReportMsg)
{
    int index = 0;
    for (const auto& unit : mode.unitMap)
    {
        //        qWarning() << allUnitParamBytes.mid(index).toHex();

        //        qWarning() <<
        //        QString("%1------------------------------------------------------------------------------------------------------------------")
        //                        .arg(unit.desc);

        index += 2;  // 跳过单元开始标识

        UnitReportMessage unitReportMsg;
        unitReportMsg.id = unit.unitCode;

        for (auto param : unit.queryResultList)
        {
            QVariant paramVal;
            if (!unpackParam(param, allUnitParamBytes, paramVal, index))
            {
                qWarning() << "存储转发解析错误点1：" << param.id;
                return false;
            }
            unitReportMsg.paramMap[param.id] = paramVal;

            //            qWarning() << param.desc << paramVal;

            int targetNo;
            QList<ParameterAttribute> targetParams;
            unit.getRecycleTargetParams(param.id, targetNo, targetParams);  // 获取循环上报参数
            if (targetParams.isEmpty())
            {
                continue;
            }

            QList<QMap<QString, QVariant>> targetRecycleParamList;
            if (paramVal.toInt() > 5)  // 有时候设备报的值很大，导致内存增长  中心数不能超过5个
            {
                qWarning() << "存储转发解析错误点2：" << paramVal.toInt();
                // return false;
            }
            for (int i = 0; i < paramVal.toInt(); ++i)  // 循环上报值，表示需要循环多少次
            {
                QMap<QString, QVariant> paramMap;
                for (const auto& targetParam : targetParams)
                {
                    if (targetParam.pid >= 15)  // 先循环处理中心相关的
                    {
                        break;
                    }

                    //                    int temp = index;

                    QVariant paramVal;
                    if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                    {
                        qWarning() << "存储转发解析错误点3：" << targetParam.id;
                        return false;
                    }
                    paramMap[targetParam.id] = paramVal;

                    //                    qWarning() << targetParam.desc << paramVal << allUnitParamBytes.mid(temp, (index - temp)).toHex() << (index
                    //                    - temp);

                    int targetNo2;
                    QList<ParameterAttribute> targetParams;
                    unit.getRecycleTargetParams(targetParam.id, targetNo2, targetParams);
                    if (targetParams.isEmpty())
                    {
                        continue;
                    }

                    QList<QMap<QString, QVariant>> targetRecycle2ParamList;
                    if (paramVal.toInt() > 64)  // 有时候设备报的值很大，导致内存增长 虚拟信道数量不能超过64个
                    {
                        qWarning() << "存储转发解析错误点4：" << paramVal.toInt();
                        // return false;
                    }
                    for (int i = 0; i < paramVal.toInt(); ++i)
                    {
                        QMap<QString, QVariant> paramMap;
                        for (const auto& targetParam : targetParams)
                        {
                            //                            int temp = index;

                            QVariant paramVal;
                            if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                            {
                                qWarning() << "存储转发解析错误点5：" << targetParam.id;
                                return false;
                            }
                            paramMap[targetParam.id] = paramVal;

                            //                            qWarning() << targetParam.desc << paramVal << allUnitParamBytes.mid(temp, (index -
                            //                            temp)).toHex() << (index - temp);
                        }
                        targetRecycle2ParamList << paramMap;
                    }
                    unitReportMsg.multiTargetParamMap3[targetNo][targetNo2] = targetRecycle2ParamList;
                }
                targetRecycleParamList << paramMap;
            }

            for (int i = 0; i < paramVal.toInt(); ++i)  // 循环上报值，表示需要循环多少次
            {
                QMap<QString, QVariant> paramMap;
                for (const auto& targetParam : targetParams)
                {
                    if (targetParam.pid < 15)
                    {
                        continue;
                    }
                    // 再循环文件相关的
                    //                    int temp = index;

                    QVariant paramVal;
                    if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                    {
                        qWarning() << "存储转发解析错误点6：" << targetParam.id;
                        return false;
                    }
                    paramMap[targetParam.id] = paramVal;

                    //                    qWarning() << targetParam.desc << paramVal << allUnitParamBytes.mid(temp, (index - temp)).toHex() << (index
                    //                    - temp);

                    int targetNo2;
                    QList<ParameterAttribute> targetParams;
                    unit.getRecycleTargetParams(targetParam.id, targetNo2, targetParams);
                    if (targetParams.isEmpty())
                    {
                        continue;
                    }

                    QList<QMap<QString, QVariant>> targetRecycle2ParamList;
                    if (paramVal.toInt() > 64)  // 有时候设备报的值很大，导致内存增长 虚拟信道数量不能超过64个
                    {
                        qWarning() << "存储转发解析错误点7：" << paramVal.toInt();
                        //  return false;
                    }
                    for (int i = 0; i < paramVal.toInt(); ++i)
                    {
                        QMap<QString, QVariant> paramMap;
                        for (const auto& targetParam : targetParams)
                        {
                            //                            int temp = index;

                            QVariant paramVal;
                            if (!unpackParam(targetParam, allUnitParamBytes, paramVal, index))
                            {
                                qWarning() << "存储转发解析错误点8：" << targetParam.id;
                                return false;
                            }
                            paramMap[targetParam.id] = paramVal;

                            //                            qWarning() << targetParam.desc << paramVal << allUnitParamBytes.mid(temp, (index -
                            //                            temp)).toHex() << (index - temp);
                        }
                        targetRecycle2ParamList << paramMap;
                    }
                    unitReportMsg.multiTargetParamMap3[targetNo][targetNo2] = targetRecycle2ParamList;
                }

                if (targetRecycleParamList.isEmpty())
                {
                    targetRecycleParamList << paramMap;
                }
                else
                {
                    for (auto paramId : paramMap.keys())
                    {
                        targetRecycleParamList[0][paramId] = paramMap.value(paramId);
                    }
                }
            }

            unitReportMsg.multiTargetParamMap2[targetNo] = targetRecycleParamList;
        }

        index += 2;  // 跳过单元结束标识

        extenStatusReportMsg.unitReportMsgMap[unitReportMsg.id] = unitReportMsg;
    }
    return true;
}
