#ifndef DEVICEPROCESSMESSAGE_H
#define DEVICEPROCESSMESSAGE_H

#include "CustomPacketMessageDefine.h"
#include "JsonHelper.h"
#include "ProtocolXmlTypeDefine.h"

struct CmdRequest
{
    //    int m_stationID;  //统一站ID
    //    int m_deviceID;   //统一DeviceID
    int m_systemNumb{ 0 };  //系统ID
    int m_deviceNumb{ 0 };  //设备ID
    int m_extenNumb{ 0 };   //分机ID

    int m_modeID{ 0 };  //当前设备模式
    int m_cmdID{ 0 };   //当前的命令ID
    QMap<QString, QVariant> m_paramdataMap;

    QMap<int, QList<QPair<QString, QVariant>>> multiParamMap;  //这个是存储动态界面的值
};

/* 用于处理客户端单元参数设置命令 */

struct MultiTargetParamRequest
{
    QMap<QString, QVariant> m_paramdataMap;  //多目标单元下面的参数
};

struct UnitParamRequest
{
    // int m_stationID; //统一站ID
    // int m_deviceID;  //统一DeviceID

    int m_systemNumb{ 0 };  //系统ID
    int m_deviceNumb{ 0 };  //设备ID
    int m_extenNumb{ 0 };   //分机ID

    int m_modeID{ 0 };  //当前设备模式
    int m_unitID{ 0 };  //当前的单元ID

    int m_validIdent{ -1 };                                    //有效标识
    QMap<QString, QVariant> m_paramdataMap;                    //这个方法只处理简单模式的数据 ，如果是复杂的模式待定
    QMap<int, MultiTargetParamRequest> m_multiTargetParamMap;  //处理多目标
};

struct StatusUnitReportingRequest
{
    QMap<QString, QVariant> m_paramdataMap;                    //这个方法只处理简单模式的数据 ，如果是复杂的模式待定
    QMap<int, MultiTargetParamRequest> m_multiTargetParamMap;  //处理多目标

    QMap<int, QList<QMap<QString, QVariant>>> multiTargetParamMap2;
    QMap<int, QMap<int, QList<QMap<QString, QVariant>>>> multiTargetParamMap3;
};

struct StatusReportingRequest
{
    int m_systemNumb{ 0 };
    int m_deviceNumb{ 0 };
    int m_extenNumb{ 0 };

    int m_modeID{ 0 };
    QMap<int, StatusUnitReportingRequest> m_paramdataMap;  //这个方法只处理简单模式的数据 ，如果是复杂的模式待定
};

struct ParamMacroRequest
{
    int m_systemNumb{ 0 };  //系统ID
    int m_deviceNumb{ 0 };  //设备ID
    int m_extenNumb{ 0 };   //分机ID

    int m_modeID{ 0 };  //当前设备模式

    int m_validIdent{ -1 };                           //有效标识
    QMap<int, UnitParamRequest> m_paramMacroInfoMap;  // key 对应单元ID，value 对应每个单元的参数
};

struct ControlCmdResponse
{
    DeviceID deviceID;                                                             // 设备ID
    int modeId{ 0 };                                                               // 模式id
    int cmdId{ 0 };                                                                // 单元或者命令的id
    DevMsgType cmdType{ DevMsgType::Unknown };                                     // 命令类型
    ControlCmdResponseType responseResult{ ControlCmdResponseType::UnknowError };  // 控制结果

    inline bool isValid() const { return deviceID.isValid(); }
};

struct CmdResult
{
    //    int m_stationID;  //统一站ID
    //    int m_deviceID;   //统一DeviceID
    int m_systemNumb{ 0 };  //系统ID
    int m_deviceNumb{ 0 };  //设备ID
    int m_extenNumb{ 0 };   //分机ID

    int m_modeID{ 0 };  //当前设备模式
    int m_cmdID{ 0 };   //当前的命令ID
    QMap<QString, QVariant> m_paramdataMap;

    QMap<int, QList<QPair<QString, QVariant>>> m_multiParamMap;
};

Q_DECLARE_METATYPE(CmdRequest)
Q_DECLARE_METATYPE(MultiTargetParamRequest)
Q_DECLARE_METATYPE(UnitParamRequest)
Q_DECLARE_METATYPE(StatusUnitReportingRequest)
Q_DECLARE_METATYPE(StatusReportingRequest)
Q_DECLARE_METATYPE(ParamMacroRequest)
Q_DECLARE_METATYPE(ControlCmdResponse)
Q_DECLARE_METATYPE(CmdResult)

#endif  // DEVICEPROCESSMESSAGE_H
