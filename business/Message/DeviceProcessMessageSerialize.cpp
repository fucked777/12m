#include "DeviceProcessMessageSerialize.h"
#include "JsonHelper.h"
#include <QDebug>
#include <QMap>
#include <QVariant>

JsonWriter& operator&(JsonWriter& self, const CmdRequest& data)
{
    self.StartObject();
    //    ar.Member("StationID") & data.m_stationID;
    //    ar.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("CmdID") & data.m_cmdID;

    self.Member("ParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    self.Member("MultiParamData");
    self.StartObject();

    for (auto key : data.multiParamMap.keys())
    {
        self.Member(QString::number(key));
        self.StartObject();
        auto list = data.multiParamMap[key];
        for (auto pair : list)
        {
            self.Member(pair.first) & pair.second;
        }
        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, CmdRequest& data)
{
    self.StartObject();
    //    ar.Member("StationID") & data.m_stationID;
    //    ar.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("CmdID") & data.m_cmdID;

    self.Member("ParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    self.Member("MultiParamData");

    self.StartObject();
    auto multkeys = self.Members();
    for (auto key : multkeys)
    {
        self.Member(key);
        self.StartObject();
        auto lists = self.Members();
        QList<QPair<QString, QVariant>> listData;
        for (auto pairKey : lists)
        {
            QVariant temp;
            self.Member(pairKey) & temp;
            QPair<QString, QVariant> tempPair;
            tempPair.first = pairKey;
            tempPair.second = temp;
            listData.append(tempPair);
        }
        self.EndObject();
        data.multiParamMap[key.toInt()] = listData;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, CmdRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const CmdRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, CmdRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const CmdRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

/* 用于处理客户端单元参数设置命令 */

JsonWriter& operator&(JsonWriter& self, const MultiTargetParamRequest& data)
{
    self.StartObject();

    self.Member("MultiTargetParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, MultiTargetParamRequest& data)
{
    self.StartObject();

    self.Member("MultiTargetParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, MultiTargetParamRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const MultiTargetParamRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, MultiTargetParamRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const MultiTargetParamRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const UnitParamRequest& data)
{
    self.StartObject();
    // self.Member("StationID") & data.m_stationID;
    // self.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("m_validIdent") & data.m_validIdent;

    self.Member("ModeID") & data.m_modeID;
    self.Member("UnitID") & data.m_unitID;
    self.Member("ParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    self.Member("MultiTargetParamData");
    self.StartObject();
    for (auto iter = data.m_multiTargetParamMap.begin(); iter != data.m_multiTargetParamMap.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, UnitParamRequest& data)
{
    self.StartObject();
    // self.Member("StationID") & data.m_stationID;
    // self.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("m_validIdent") & data.m_validIdent;

    self.Member("ModeID") & data.m_modeID;
    self.Member("UnitID") & data.m_unitID;
    self.Member("ParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    self.Member("MultiTargetParamData");
    self.StartObject();
    keys = self.Members();
    for (auto key : keys)
    {
        MultiTargetParamRequest temp;
        self.Member(key) & temp;
        data.m_multiTargetParamMap[key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
JsonWriter& operator&&(JsonWriter& self, const UnitParamRequest& data)
{
    self.StartObject();

    self.Member("ParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    self.Member("MultiTargetParamData");
    self.StartObject();
    for (auto iter = data.m_multiTargetParamMap.begin(); iter != data.m_multiTargetParamMap.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&&(JsonReader& self, UnitParamRequest& data)
{
    self.StartObject();

    self.Member("ParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    self.Member("MultiTargetParamData");

    self.StartObject();
    keys = self.Members();
    for (auto key : keys)
    {
        MultiTargetParamRequest temp;
        self.Member(key) & temp;
        data.m_multiTargetParamMap[key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, UnitParamRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const UnitParamRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, UnitParamRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const UnitParamRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
/* 用于状态数据上报给客户端显示*/

JsonWriter& operator&(JsonWriter& self, const StatusUnitReportingRequest& data)
{
    self.StartObject();

    self.Member("UnitParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    self.Member("MultiTargetParamData");
    self.StartObject();
    for (auto iter = data.m_multiTargetParamMap.begin(); iter != data.m_multiTargetParamMap.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StatusUnitReportingRequest& data)
{
    self.StartObject();

    self.Member("UnitParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    self.Member("MultiTargetParamData");
    self.StartObject();
    keys = self.Members();
    for (auto key : keys)
    {
        MultiTargetParamRequest temp;
        self.Member(key) & temp;
        data.m_multiTargetParamMap[key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, StatusUnitReportingRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const StatusUnitReportingRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, StatusUnitReportingRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const StatusUnitReportingRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const StatusReportingRequest& data)
{
    self.StartObject();
    // ar.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("ParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StatusReportingRequest& data)
{
    self.StartObject();
    // ar.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("ParamData");

    self.StartObject();
    auto keys = self.Members();

    for (auto key : keys)
    {
        StatusUnitReportingRequest temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, StatusReportingRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const StatusReportingRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, StatusReportingRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const StatusReportingRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ParamMacroRequest& data)
{
    self.StartObject();

    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("m_validIdent") & data.m_validIdent;

    self.Member("ModeID") & data.m_modeID;

    self.Member("MacroParamData");
    self.StartObject();
    for (auto iter = data.m_paramMacroInfoMap.begin(); iter != data.m_paramMacroInfoMap.end(); ++iter)
    {
        self.Member(QString::number(iter.key())) & iter.value();
    }

    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ParamMacroRequest& data)
{
    self.StartObject();

    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("m_validIdent") & data.m_validIdent;

    self.Member("ModeID") & data.m_modeID;

    self.Member("MacroParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        UnitParamRequest temp;
        self.Member(key) && temp;
        data.m_paramMacroInfoMap[key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ParamMacroRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ParamMacroRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ParamMacroRequest& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ParamMacroRequest& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ControlCmdResponse& data)
{
    self.StartObject();

    int deviceId = 0;
    data.deviceID >> deviceId;
    self.Member("DeviceID") & deviceId;

    self.Member("CmdId") & data.cmdId;

    int cmdType = (int)data.cmdType;
    self.Member("CmdType") & cmdType;

    int responseResult = (int)data.responseResult;
    self.Member("ResponseResult") & responseResult;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ControlCmdResponse& data)
{
    self.StartObject();

    int deviceId = 0;
    self.Member("DeviceID") & deviceId;
    data.deviceID = DeviceID(deviceId);

    self.Member("CmdId") & data.cmdId;

    int cmdType = 0;
    self.Member("CmdType") & cmdType;
    data.cmdType = (DevMsgType)cmdType;

    int responseResult = 0;
    self.Member("ResponseResult") & responseResult;
    data.responseResult = (ControlCmdResponseType)responseResult;

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ControlCmdResponse& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ControlCmdResponse& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ControlCmdResponse& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ControlCmdResponse& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const CmdResult& data)
{
    self.StartObject();
    // self.Member("StationID") & data.m_stationID;
    // self.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("CmdID") & data.m_cmdID;

    self.Member("ParamData");

    self.StartObject();
    for (auto iter = data.m_paramdataMap.begin(); iter != data.m_paramdataMap.end(); ++iter)
    {
        self.Member(iter.key()) & iter.value();
    }
    self.EndObject();

    self.Member("MultiParamData");
    self.StartObject();

    for (auto key : data.m_multiParamMap.keys())
    {
        self.Member(QString::number(key));
        self.StartObject();
        auto list = data.m_multiParamMap[key];
        for (auto pair : list)
        {
            self.Member(pair.first) & pair.second;
        }
        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, CmdResult& data)
{
    self.StartObject();
    // self.Member("StationID") & data.m_stationID;
    // self.Member("DeviceID") & data.m_deviceID;
    self.Member("SystemNumb") & data.m_systemNumb;
    self.Member("DeviceNumb") & data.m_deviceNumb;
    self.Member("ExtenNumb") & data.m_extenNumb;
    self.Member("ModeID") & data.m_modeID;
    self.Member("CmdID") & data.m_cmdID;

    self.Member("ParamData");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QVariant temp;
        self.Member(key) & temp;
        data.m_paramdataMap[key] = temp;
    }
    self.EndObject();

    self.Member("MultiParamData");

    self.StartObject();
    auto multkeys = self.Members();
    for (auto key : multkeys)
    {
        self.Member(key);
        self.StartObject();
        auto lists = self.Members();
        QList<QPair<QString, QVariant>> listData;
        for (auto pairKey : lists)
        {
            QVariant temp;
            self.Member(pairKey) & temp;
            QPair<QString, QVariant> tempPair;
            tempPair.first = pairKey;
            tempPair.second = temp;
            listData.append(tempPair);
        }
        self.EndObject();
        data.m_multiParamMap[key.toInt()] = listData;
    }
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, CmdResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const CmdResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, CmdResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const CmdResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

QString DeviceProcessHelper::getCmdResponseRedisKey(const DeviceID& deviceID, qint32 cmdOrUnitID)
{
    return QString("Responce_%1_%2").arg(deviceID.toHex()).arg(static_cast<quint32>(cmdOrUnitID), 0, 16);
}

QString DeviceProcessHelper::getCmdResponseRedisKey(const MessageAddress& addr, qint32 cmdOrUnitID)
{
    auto deviceID = DeviceID(addr.systemNumb, addr.deviceNumb, addr.extenNumb);
    return getCmdResponseRedisKey(deviceID, cmdOrUnitID);
}

QString DeviceProcessHelper::getCmdResultRedisKey(const DeviceID& deviceID, qint32 cmdOrUnitID)
{
    return QString("Result_%1_%2").arg(deviceID.toHex()).arg(static_cast<quint32>(cmdOrUnitID), 0, 16);
}
QString DeviceProcessHelper::getCmdResultRedisKey(const MessageAddress& addr, qint32 cmdOrUnitID)
{
    auto deviceID = DeviceID(addr.systemNumb, addr.deviceNumb, addr.extenNumb);
    return getCmdResultRedisKey(deviceID, cmdOrUnitID);
}
