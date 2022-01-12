#include "ProcessControlCmdPacker.h"

#include "CConverter.h"
#include "GlobalData.h"

ProcessControlCmdPacker::ProcessControlCmdPacker(QObject* parent)
    : BasePacker(parent)
{
}

ProcessControlCmdPacker::~ProcessControlCmdPacker() {}

DevMsgType ProcessControlCmdPacker::messageType() const { return DevMsgType::ProcessControlCmd; }

bool ProcessControlCmdPacker::packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const
{
    const auto& targetAddr = message.header.targetAddr;
    const auto& processCtrlCmdMsg = message.processCtrlCmdMsg;

    // 获取命令参数
    CmdAttribute cmdAttr = GlobalData::getCmd(targetAddr, processCtrlCmdMsg.mode, processCtrlCmdMsg.cmdId);
    if (!cmdAttr.isValid())
    {
        DeviceID deviceID(targetAddr.systemNumb, targetAddr.deviceNumb, targetAddr.extenNumb);
        int intDeviceId;
        deviceID >> intDeviceId;
        errorMsg = QString("未匹配到需要下发的过程控制命令，设备ID：0x%1，模式id：0x%2，命令id：%3")
                       .arg(QString::number(intDeviceId, 16))
                       .arg(QString::number(processCtrlCmdMsg.mode, 16).toUpper())
                       .arg(processCtrlCmdMsg.cmdId);
        return false;
    }

    bodyBytes.resize(0);
    // 添加过程命令标识
    QByteArray cmdIdentdBytes;
    if (!packProcessCtrlCmdIdent(targetAddr.systemNumb, targetAddr.deviceNumb, processCtrlCmdMsg.cmdId, cmdIdentdBytes))
    {
        return false;
    }
    bodyBytes.append(cmdIdentdBytes);

    // 添加顺序号
    bodyBytes.append(CConverter::toByteArray<ushort>(0));

    // 添加普通命令参数
    QByteArray paramBytes;
    if (!packParams(cmdAttr.requestList, processCtrlCmdMsg.settingParamMap, paramBytes))
    {
        return false;
    }
    bodyBytes.append(paramBytes);

    // 添加动态命令参数
    for (auto recycleId : processCtrlCmdMsg.multiParamMap.keys())
    {
        auto recycleParamList = cmdAttr.requestRecycle.value(recycleId);
        if (recycleParamList.isEmpty())
        {
            return false;
        }

        auto recycleParamValueList = processCtrlCmdMsg.multiParamMap.value(recycleId);

        //        auto recycleNum = recycleParamValueList.size() / recycleParamList.size();
        //        for (int i = 0; i < recycleNum; ++i)
        //        {
        //            QMap<QString, QVariant> paramMap;
        //            auto tempParamList = recycleParamValueList.mid(i * recycleParamList.size(), recycleParamList.size());
        //            for (auto paramPair : tempParamList)
        //            {
        //                paramMap.clear();
        //                paramMap.insert(paramPair.first, paramPair.second);
        //            }

        //            QByteArray paramBytes;
        //            if (!packParams(recycleParamList, paramMap, paramBytes))
        //            {
        //                return false;
        //            }
        //            bodyBytes.append(paramBytes);
        //        }

        auto recycleNum = recycleParamValueList.size();
        for (int i = 0; i < recycleNum; ++i)
        {
            QMap<QString, QVariant> paramMap;
            auto tempParamList = recycleParamValueList.at(i);
            //现在这种动态命令都是按顺序来组包，暂不用管一个recycle里面有几个参数，按顺序组下去就行

            QList<ParameterAttribute> requestRecycleSet;

            for (auto attr : recycleParamList)  //每次只组一个参数，去掉不同名的参数
            {
                if (tempParamList.first.indexOf(attr.id) >= 0)
                {
                    requestRecycleSet.append(attr);
                    paramMap.insert(attr.id, tempParamList.second);
                }
            }

            QByteArray paramBytes;
            if (!packParams(requestRecycleSet, paramMap, paramBytes))
            {
                return false;
            }
            bodyBytes.append(paramBytes);
        }
    }

    return true;
}

bool ProcessControlCmdPacker::packProcessCtrlCmdIdent(int systemId, int deviceId, int cmdId, QByteArray& cmdIdentBytes) const
{
    char systemCode = 0x00;

    switch (systemId)
    {
    case 0x0: systemCode = 'M'; break;  // 监控
    case 0x1: systemCode = 'A'; break;  // 天伺馈
    case 0x2:
    {
        switch (deviceId)
        {
        case 0x0:
        case 0x1:
        case 0x3: systemCode = 'H'; break;  // 高功放
        case 0x4:
        case 0x5: systemCode = 'T'; break;  // 小信号
        default: return false;
        }
    }
    break;
    case 0x3: systemCode = 'R'; break;  // 高频接收
    case 0x4: systemCode = 'B'; break;  // 基带
    case 0x5: systemCode = 'F'; break;  // 时频
    case 0x6: systemCode = 'C'; break;  // 标校
    case 0x7: systemCode = 'F'; break;  // 数字化前端
    case 0x8:
    {
        switch (deviceId)
        {
        case 0x0: systemCode = 'D'; break;  // 数据传输
        case 0x1: systemCode = 'P'; break;  // 记录
        default: return false;
        }
    }
    break;
    default: return false;
    }

    cmdIdentBytes.resize(0);
    quint16 cmdIdent = cmdId | (systemCode << 8);

    cmdIdentBytes.append(CConverter::toByteArray<ushort>(cmdIdent));
    return true;
}
