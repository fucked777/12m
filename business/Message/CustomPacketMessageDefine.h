#ifndef CUSTOMPACKETMESSAGEDEFINE_H
#define CUSTOMPACKETMESSAGEDEFINE_H

#include "DevProtocol.h"
#include <QtCore>

// 每个单元上报消息
struct UnitReportMessage
{
    int id{ 0 };                                             // 单元id
    QMap<QString, QVariant> paramMap;                        // 单目标 QMap<参数id， 上报值>
    QMap<int, QMap<QString, QVariant>> multiTargetParamMap;  // 多目标 QMap<目标号, QMap<参数id， 上报值>>

    QMap<int, QList<QMap<QString, QVariant>>> multiTargetParamMap2;
    QMap<int, QMap<int, QList<QMap<QString, QVariant>>>> multiTargetParamMap3;
    QMap<QVariant, QList<QVariant>> dteTaskBZAndCenterBZMap;  //专用于DTE一个任务包含多个中心标识的map
};

// 分机状态上报信息
struct ExtensionStatusReportMessage
{
    int modeId{ -1 };                               // 当前模式
    QMap<int, UnitReportMessage> unitReportMsgMap;  // QMap<单元id, 单元上报信息>
};

// 控制命令响应消息
struct ControlCmdResponseMessage
{
    DevMsgType msgType{ DevMsgType::Unknown };  // 消息类别
    quint32 cmdId{ 0 };                         // 单元或者命令的ID
                                                // 过程控制命令：低2字节填写过程控制命令标识，高2字节填0；
                                                // 单元参数设置命令：最低字节填写单元标识，高字节填0；
                                                // 组参数设置命令：填写“全1”

    quint16 orderNum{ 0 };                                                 // 顺序号
    ControlCmdResponseType result{ ControlCmdResponseType::UnknowError };  // 控制结果
};

// 控制结果上报信息
struct ControlResultReportMessage
{
    quint8 id{ 0 };  // 命令号

    QMap<QString, QVariant> paramMap;  // QMap<参数id, 上报值>

    QMap<int, QList<QPair<QString, QVariant>>> multiParamMap;
};

// 解包消息
struct UnpackMessage
{
    DevMessageHeader header;

    ExtensionStatusReportMessage extenStatusReportMsg;  // 分机状态上报消息 QMap<单元编号, 单元上报信息>
    ControlCmdResponseMessage controlCmdRespMsg;        // 控制命令响应消息
    ControlResultReportMessage controlResultReportMsg;  // 控制结果上报消息

    QByteArray rawData;  // 原始数据
};

/////////////////////////////////////////////////////////////////////

// 过程控制命令消息
struct ProcessControlCmdMessage
{
    quint16 cmdId{ 0 };                                        // 命令号
    int mode{ -1 };                                            // 当前模式
    QMap<QString, QVariant> settingParamMap;                   // 设置参数 QMap<参数id, 参数值>
    QMap<int, QList<QPair<QString, QVariant>>> multiParamMap;  // 这个是存储动态界面的值   QMap<目标id, QList<QPair<参数id, 参数值>>>
};

// 单元参数设置消息
struct UnitParamSetMessage
{
    quint8 unitId{ 0 };                                      // 单元id
    int mode{ -1 };                                          // 当前模式
    int channelValidIdent{ 0 };                              // 有效目标标识
    QMap<QString, QVariant> settingParamMap;                 // 设置参数 QMap<参数id, 参数值>
    QMap<int, QMap<QString, QVariant>> multiTargetParamMap;  // 多目标参数 QMap<目标号, QMap<参数id， 参数值>>
};

// 组参数设置命令消息
struct GroupParamSetMessage
{
    int modeId{ -1 };                                   // 模式id
    QMap<int, UnitParamSetMessage> unitParamSetMsgMap;  // 组参数单元 QMap<单元id, 组参数单元>
};

// 状态查询命令消息
struct StatusQueryCmdMessage
{
};

// 组包消息
struct PackMessage
{
    DevMessageHeader header;  // PDXP消息头

    UnitParamSetMessage unitParamSetMsg;         // 单元参数设置消息
    ProcessControlCmdMessage processCtrlCmdMsg;  // 过程控制命令消息
    GroupParamSetMessage groupParamSetMsg;       // 组参数设置命令消息
    StatusQueryCmdMessage statusQueryCmdMsg;     // 状态查询命令消息
};

using DeviceReportMessageMap = QMap<QString, ExtensionStatusReportMessage>;

Q_DECLARE_METATYPE(UnitReportMessage)
Q_DECLARE_METATYPE(ExtensionStatusReportMessage)

#endif  // CUSTOMPACKETMESSAGEDEFINE_H
