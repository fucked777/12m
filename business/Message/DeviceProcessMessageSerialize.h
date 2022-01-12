#ifndef DEVICEPROCESSMESSAGESERIALIZE_H
#define DEVICEPROCESSMESSAGESERIALIZE_H

#include "DeviceProcessMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const CmdRequest& data);
JsonReader& operator&(JsonReader& self, CmdRequest& data);
QByteArray& operator>>(QByteArray& self, CmdRequest& data);
QByteArray& operator<<(QByteArray& self, const CmdRequest& data);
QString& operator<<(QString& self, const CmdRequest& data);
QString& operator>>(QString& self, CmdRequest& data);

JsonWriter& operator&(JsonWriter& self, const MultiTargetParamRequest& data);
JsonReader& operator&(JsonReader& self, MultiTargetParamRequest& data);
QByteArray& operator>>(QByteArray& self, MultiTargetParamRequest& data);
QByteArray& operator<<(QByteArray& self, const MultiTargetParamRequest& data);
QString& operator<<(QString& self, const MultiTargetParamRequest& data);
QString& operator>>(QString& self, MultiTargetParamRequest& data);

JsonWriter& operator&(JsonWriter& self, const UnitParamRequest& data);
JsonReader& operator&(JsonReader& self, UnitParamRequest& data);
JsonWriter& operator&&(JsonWriter& self, const UnitParamRequest& data);
JsonReader& operator&&(JsonReader& self, UnitParamRequest& data);
QByteArray& operator>>(QByteArray& self, UnitParamRequest& data);
QByteArray& operator<<(QByteArray& self, const UnitParamRequest& data);
QString& operator<<(QString& self, const UnitParamRequest& data);
QString& operator>>(QString& self, UnitParamRequest& data);

JsonWriter& operator&(JsonWriter& self, const StatusUnitReportingRequest& data);
JsonReader& operator&(JsonReader& self, StatusUnitReportingRequest& data);
QByteArray& operator>>(QByteArray& self, StatusUnitReportingRequest& data);
QByteArray& operator<<(QByteArray& self, const StatusUnitReportingRequest& data);
QString& operator<<(QString& self, const StatusUnitReportingRequest& data);
QString& operator>>(QString& self, StatusUnitReportingRequest& data);

JsonWriter& operator&(JsonWriter& self, const StatusReportingRequest& data);
JsonReader& operator&(JsonReader& self, StatusReportingRequest& data);
QByteArray& operator>>(QByteArray& self, StatusReportingRequest& data);
QByteArray& operator<<(QByteArray& self, const StatusReportingRequest& data);
QString& operator<<(QString& self, const StatusReportingRequest& data);
QString& operator>>(QString& self, StatusReportingRequest& data);

JsonWriter& operator&(JsonWriter& self, const ParamMacroRequest& data);
JsonReader& operator&(JsonReader& self, ParamMacroRequest& data);
QByteArray& operator>>(QByteArray& self, ParamMacroRequest& data);
QByteArray& operator<<(QByteArray& self, const ParamMacroRequest& data);
QString& operator<<(QString& self, const ParamMacroRequest& data);
QString& operator>>(QString& self, ParamMacroRequest& data);

JsonWriter& operator&(JsonWriter& self, const ControlCmdResponse& data);
JsonReader& operator&(JsonReader& self, ControlCmdResponse& data);
QByteArray& operator>>(QByteArray& self, ControlCmdResponse& data);
QByteArray& operator<<(QByteArray& self, const ControlCmdResponse& data);
QString& operator<<(QString& self, const ControlCmdResponse& data);
QString& operator>>(QString& self, ControlCmdResponse& data);

JsonWriter& operator&(JsonWriter& self, const CmdResult& data);
JsonReader& operator&(JsonReader& self, CmdResult& data);
QByteArray& operator>>(QByteArray& self, CmdResult& data);
QByteArray& operator<<(QByteArray& self, const CmdResult& data);
QString& operator<<(QString& self, const CmdResult& data);
QString& operator>>(QString& self, CmdResult& data);

class DeviceProcessHelper
{
public:
    /*
     * 获取命令相应的key
     * 此key用于本地缓存或者redis缓存
     * 单元命令 cmdOrUnitID 是单元号
     * 过程命令 cmdOrUnitID 是命令号
     * 组参数   cmdOrUnitID 是全1
     */
    static QString getCmdResponseRedisKey(const DeviceID& deviceID, qint32 cmdOrUnitID);
    static QString getCmdResponseRedisKey(const MessageAddress& addr, qint32 cmdOrUnitID);
    static QString getCmdResultRedisKey(const DeviceID& deviceID, qint32 cmdOrUnitID);
    static QString getCmdResultRedisKey(const MessageAddress& addr, qint32 cmdOrUnitID);
};

#endif  // DEVICEPROCESSMESSAGESERIALIZE_H
