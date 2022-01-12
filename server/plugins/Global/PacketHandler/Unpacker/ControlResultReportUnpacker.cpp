#include "ControlResultReportUnpacker.h"

#include "CConverter.h"
#include "GlobalData.h"

ControlResultReportUnpacker::ControlResultReportUnpacker(QObject* parent)
    : BaseUnpacker(parent)
{
}
ControlResultReportUnpacker::~ControlResultReportUnpacker() {}

DevMsgType ControlResultReportUnpacker::messageType() const { return DevMsgType::ControlResultReport; }

bool ControlResultReportUnpacker::unpackBody(const QByteArray& bodyBytes, UnpackMessage& message)
{
    const auto& sourceAddr = message.header.sourceAddr;
    ControlResultReportMessage& ctrlResReportMsg = message.controlResultReportMsg;

    // 解析过程命令标识
    QByteArray cmdHex = bodyBytes.mid(0, 1);
    ctrlResReportMsg.id = ~(cmdHex[0] & 0xff);  //命令号按位取反

    // 通过全局获取设备模式
    int modeId = GlobalData::getOnlineDeviceModeId(sourceAddr.systemNumb, sourceAddr.deviceNumb, sourceAddr.extenNumb);
    if (modeId == -1)
    {
        return false;
    }

    // 获取命令参数
    CmdAttribute cmdAttr = GlobalData::getCmd(sourceAddr, modeId, ctrlResReportMsg.id);
    if (!cmdAttr.isValid())
    {
        qDebug() << "获取命令数据失败，命令号：" << ctrlResReportMsg.id;
        return false;
    }

    // 解析上报参数
    int index = 0;
    QByteArray paramBytes = bodyBytes.mid(2);

    if (!cmdAttr.resultRecycle.isEmpty())
    {
        return unpackMultiBody(paramBytes, cmdAttr, ctrlResReportMsg);
    }

    return unpackParams(cmdAttr.resultList, paramBytes, ctrlResReportMsg.paramMap, index);
}

bool ControlResultReportUnpacker::unpackMultiBody(const QByteArray& cmdParamsBytes, CmdAttribute& cmdAttr,
                                                  ControlResultReportMessage& controlResultReportMsg)
{
    int index = 0;

    for (auto attr : cmdAttr.resultList)
    {
        QVariant paramVal;
        if (!unpackParam(attr, cmdParamsBytes, paramVal, index))
        {
            qDebug() << "动态命令解析错误1:" << attr.desc;
            return false;
        }

        controlResultReportMsg.paramMap[attr.id] = paramVal;

        int targetFirst;
        QList<ParameterAttribute> targetParams;
        cmdAttr.getResultRecycleTargetParams(attr.id, targetFirst, targetParams);
        if (targetParams.isEmpty())
        {
            continue;
        }
        if (paramVal.toInt() > 150)  //有时候设备报的值很大，导致内存增长
        {
            qDebug() << "动态命令解析错误2:" << attr.desc;
            return false;
        }

        QList<QPair<QString, QVariant>> pairList;
        for (int i = 0; i < paramVal.toInt(); ++i)
        {
            QPair<QString, QVariant> paramPair;
            for (auto& targetParam : targetParams)
            {
                QVariant targetParamVal;
                if (!unpackParam(targetParam, cmdParamsBytes, targetParamVal, index))
                {
                    qDebug() << "动态命令解析错误3:" << targetParam.id;
                    return false;
                }

                paramPair.first = targetParam.id + QString::number(i + 1);  //因为ID都一样，避免序列化覆盖，都加上i
                paramPair.second = targetParamVal;
                pairList.append(paramPair);
            }
        }

        controlResultReportMsg.multiParamMap[targetFirst] = pairList;
    }

    return true;
}
