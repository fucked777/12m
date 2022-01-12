#include "StatusPersistenceMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const ParamStatus& data)
{
    self.StartObject();

    self.Member("DataType") & data.dataType;
    self.Member("ParName") & data.parName;
    self.Member("Step") & data.step;
    self.Member("Value") & data.value.toString();
    self.Member("Unit") & data.unit;
    self.Member("MinValue") & data.minValue.toString();
    self.Member("MaxValue") & data.maxValue.toString();
    self.Member("EnumValueDesc") & data.enumValueDesc;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ParamStatus& data)
{
    self.StartObject();

    self.Member("DataType") & data.dataType;
    self.Member("ParName") & data.parName;
    self.Member("Step") & data.step;
    self.Member("Value") & data.value;
    self.Member("Unit") & data.unit;
    self.Member("MinValue") & data.minValue;
    self.Member("MaxValue") & data.maxValue;
    self.Member("EnumValueDesc") & data.enumValueDesc;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ParamStatus& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ParamStatus& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ParamStatus& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ParamStatus& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const UnitStatusParam& data)
{
    self.StartObject();

    for (auto key : data.paramStatusMap.keys())
    {
        auto value = data.paramStatusMap.value(key);
        self.Member(key) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, UnitStatusParam& data)
{
    self.StartObject();

    auto keys = self.Members();
    for (auto key : keys)
    {
        ParamStatus temp;
        self.Member(key) & temp;
        data.paramStatusMap[key] = temp;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, UnitStatusParam& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const UnitStatusParam& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, UnitStatusParam& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const UnitStatusParam& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const UnitStatusParamMap& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(key) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, UnitStatusParamMap& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        UnitStatusParam temp;
        self.Member(key) & temp;
        data[key] = temp;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, UnitStatusParamMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const UnitStatusParamMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, UnitStatusParamMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const UnitStatusParamMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceStatusData& data)
{
    self.StartObject();

    int deviceId = 0;
    data.deviceID >> deviceId;
    self.Member("deviceid") & deviceId;

    self.Member("devicename") & data.deviceName;
    self.Member("modeid") & data.modeId;
    self.Member("modelname") & data.modelName;
    self.Member("proname") & data.proName;

    self.Member("upars") & data.unitParamMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceStatusData& data)
{
    self.StartObject();

    int deviceId = 0;
    self.Member("deviceid") & deviceId;
    data.deviceID = DeviceID(deviceId);

    self.Member("devicename") & data.deviceName;
    self.Member("modeid") & data.modeId;

    self.Member("modelname") & data.modelName;
    self.Member("proname") & data.proName;

    self.Member("upars") & data.unitParamMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceStatusData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceStatusData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceStatusData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceStatusData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QStringList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QStringList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    QString info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        data.append(info);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QStringList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QStringList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QStringList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QStringList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceStatusDataCondition& data)
{
    self.StartObject();

    self.Member("devices") & data.devices;
    self.Member("units") & data.units;
    self.Member("sids") & data.sids;

    self.Member("start_time") & data.start_time;
    self.Member("end_time") & data.end_time;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceStatusDataCondition& data)
{
    self.StartObject();

    self.Member("devices") & data.devices;
    self.Member("units") & data.units;
    self.Member("sids") & data.sids;

    self.Member("start_time") & data.start_time;
    self.Member("end_time") & data.end_time;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceStatusDataCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceStatusDataCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceStatusDataCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceStatusDataCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceStatusLogData& data)
{
    self.StartObject();

    self.Member("id") & data.id;
    self.Member("createTime") & data.createTime;
    self.Member("deviceId") & data.deviceId;
    self.Member("unitId") & data.unitId;
    self.Member("param") & data.param;
    self.Member("paramValue") & data.paramValue;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceStatusLogData& data)
{
    self.StartObject();

    self.Member("id") & data.id;
    self.Member("createTime") & data.createTime;
    self.Member("deviceId") & data.deviceId;
    self.Member("unitId") & data.unitId;
    self.Member("param") & data.param;
    self.Member("paramValue") & data.paramValue;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceStatusLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceStatusLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceStatusLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceStatusLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceStatusLogDataList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }

    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, DeviceStatusLogDataList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        DeviceStatusLogData item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, DeviceStatusLogDataList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceStatusLogDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceStatusLogDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceStatusLogDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
