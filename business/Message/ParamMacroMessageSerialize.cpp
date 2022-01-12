#include "ParamMacroMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const ParamMacroData& data)
{
    self.StartObject();

    self.Member("Name") & data.name;
    self.Member("TaskCode") & data.taskCode;
    self.Member("WorkMode") & data.workMode;
    self.Member("Desc") & data.desc;

    self.Member("ModeParamMap") & data.modeParamMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ParamMacroData& data)
{
    self.StartObject();

    self.Member("Name") & data.name;
    self.Member("TaskCode") & data.taskCode;
    self.Member("WorkMode") & data.workMode;
    self.Member("Desc") & data.desc;

    self.Member("ModeParamMap") & data.modeParamMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ParamMacroData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ParamMacroData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ParamMacroData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ParamMacroData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ParamMacroModeData& data)
{
    self.StartObject();

    self.Member("PointFreqParamMap");

    self.StartObject();
    for (auto pointFreqNo : data.pointFreqParamMap.keys())
    {
        auto value = data.pointFreqParamMap.value(pointFreqNo);
        self.Member(QString::number(pointFreqNo)) & value;
    }
    self.EndObject();

    self.Member("DeviceParamMap") & data.deviceParamMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ParamMacroModeData& data)
{
    self.StartObject();

    self.Member("PointFreqParamMap");
    self.StartObject();
    auto pointFreqNos = self.Members();
    for (auto pointFreqNo : pointFreqNos)
    {
        QMap<DeviceID, DeviceData> temp;
        self.Member(pointFreqNo) & temp;

        data.pointFreqParamMap[pointFreqNo.toInt()] = temp;
    }
    self.EndObject();

    self.Member("DeviceParamMap") & data.deviceParamMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ParamMacroModeData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ParamMacroModeData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ParamMacroModeData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ParamMacroModeData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<DeviceID, DeviceData>& data)
{
    self.StartObject();
    for (auto deviceID : data.keys())
    {
        auto value = data.value(deviceID);
        self.Member(deviceID.toHex()) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<DeviceID, DeviceData>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto deviceID : keys)
    {
        DeviceData temp;
        self.Member(deviceID) & temp;
        data[DeviceID::fromHex(deviceID)] = temp;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<DeviceID, DeviceData>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<DeviceID, DeviceData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<DeviceID, DeviceData>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QMap<DeviceID, DeviceData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const DeviceData& data)
{
    self.StartObject();

    self.Member("UnitParamMap") & data.unitParamMap;

    self.Member("DynamicParamMap");
    self.StartObject();
    for (auto dynamicGroupId : data.dynamicParamMap.keys())
    {
        self.Member(dynamicGroupId);

        self.StartObject();
        auto unitDynamicMap = data.dynamicParamMap.value(dynamicGroupId);
        for (auto unitId : unitDynamicMap.keys())
        {
            auto value = unitDynamicMap.value(unitId);
            self.Member(QString::number(unitId)) & value;
        }
        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceData& data)
{
    self.StartObject();

    self.Member("UnitParamMap") & data.unitParamMap;

    self.Member("DynamicParamMap");
    self.StartObject();
    auto keys = self.Members();
    for (auto dynamicGroupId : keys)
    {
        self.Member(dynamicGroupId);
        self.StartObject();

        auto unitIds = self.Members();
        for (auto unitId : unitIds)
        {
            QMap<int, QMap<QString, QVariant>> temp;
            self.Member(unitId) & temp;

            data.dynamicParamMap[dynamicGroupId][unitId.toInt()] = temp;
        }

        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const DeviceData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number((int)key)) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        ParamMacroModeData temp;
        self.Member(key) & temp;
        data[(SystemWorkMode)(key.toInt())] = temp;
    }
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<SystemWorkMode, ParamMacroModeData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, QMap<QString, QVariant>>& data)
{
    self.StartObject();
    for (auto key1 : data.keys())
    {
        self.Member(QString::number(key1));

        self.StartObject();
        auto paramMap = data.value(key1);
        for (auto key2 : paramMap.keys())
        {
            self.Member(key2) & paramMap.value(key2).toString();
        }

        self.EndObject();
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, QMap<QString, QVariant>>& data)
{
    self.StartObject();
    auto key1s = self.Members();
    for (auto key1 : key1s)
    {
        self.Member(key1);

        self.StartObject();

        QMap<QString, QVariant> paramMap;
        auto key2s = self.Members();
        for (auto key2 : key2s)
        {
            QString value;
            self.Member(key2) & value;

            paramMap[key2] = value;
        }

        data[key1.toInt()] = paramMap;

        self.EndObject();
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, QMap<QString, QVariant>>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, QMap<QString, QVariant>>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, QMap<QString, QVariant>>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QMap<int, QMap<QString, QVariant>>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ParamMacroDataList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ParamMacroDataList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ParamMacroData info;
        self& info;
        data.append(info);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ParamMacroDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ParamMacroDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ParamMacroDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ParamMacroDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ParamMacroDataMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ParamMacroDataMap& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ParamMacroData info;
        self& info;
        data.insert(info.taskCode, info);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ParamMacroDataMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ParamMacroDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ParamMacroDataMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ParamMacroDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
