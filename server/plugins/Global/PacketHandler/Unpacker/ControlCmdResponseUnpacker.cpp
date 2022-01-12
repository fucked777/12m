#include "ControlCmdResponseUnpacker.h"

#include "CConverter.h"

ControlCmdResponseUnpacker::ControlCmdResponseUnpacker(QObject* parent)
    : BaseUnpacker(parent)
{
}
ControlCmdResponseUnpacker::~ControlCmdResponseUnpacker() {}

DevMsgType ControlCmdResponseUnpacker::messageType() const { return DevMsgType::ControlCmdResponse; }

bool ControlCmdResponseUnpacker::unpackBody(const QByteArray& bodyBytes, UnpackMessage& message)
{
    if (bodyBytes.length() != 11)
        return false;

    int index = 0;
    ControlCmdResponseMessage& ctrlCmdRespMsg = message.controlCmdRespMsg;

    // 消息类型
    ctrlCmdRespMsg.msgType = DevMsgType(CConverter::fromByteArray<uint>(bodyBytes.mid(index, DevProtocolBIDLength::value)));
    index += DevProtocolBIDLength::value;

    // 标识
    QByteArray identBytes = bodyBytes.mid(index, 4);
    index += 4;

    // 判断消息类型与标识是否与协议一致
    switch (ctrlCmdRespMsg.msgType)
    {
    case DevMsgType::ProcessControlCmd:    // 过程控制命令：低2字节填写过程控制命令标识，高2字节填0
    case DevMsgType::UnitParameterSetCmd:  // 单元参数设置命令：最低字节填写单元标识，高字节填0
    {
        //        if (CConverter::fromByteArray<ushort>(identBytes.mid(2)) != 0) // 4426Ka30W测控功放 输出功率去向响应有问题，这里不判断
        //        {
        //            return false;
        //        }
        ctrlCmdRespMsg.cmdId = CConverter::fromByteArray<ushort>(identBytes.mid(0, 1));  // 单元id或者命令id
    }
    break;
    case DevMsgType::GroupParamSetCmd:  // 组参数设置命令：填写“全1”
    {
        //        if (CConverter::fromByteArray<qulonglong>(identBytes) != 0xFFFFFFFF)
        //        {
        //            return false;
        //        }
        ctrlCmdRespMsg.cmdId = 0xFFFFFFFF;
    }
    break;
    default: return false;
    }
    // 顺序号
    ctrlCmdRespMsg.orderNum = CConverter::fromByteArray<ushort>(bodyBytes.mid(0, 2));
    index += 2;

    // 控制结果
    ctrlCmdRespMsg.result = ControlCmdResponseType(CConverter::fromByteArray<ushort>(bodyBytes.mid(index, 1)));

    return true;
}
