#include "ResourceRestructuringMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceID>& data)
{
    self.StartObject();
    int deviceId;
    for (auto key : data.keys())
    {
        auto value = data.value(key);
        value >> deviceId;
        self.Member(QString::number(key)) & deviceId;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, DeviceID>& data)
{
    self.StartObject();
    auto keys = self.Members();
    int deviceId;
    for (auto key : keys)
    {
        QString tempKey = key;
        self.Member(tempKey) & deviceId;

        DeviceID temp;
        temp << deviceId;
        data[tempKey.toInt()] = temp;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, DeviceID>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceID>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, DeviceID>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QMap<int, DeviceID>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringData& data)
{
    self.StartObject();

    self.Member("ResourceRestructuringID") & data.resourceRestructuringID;
    self.Member("Name") & data.name;

    int workMode = (int)data.workMode;
    self.Member("WorkMode") & workMode;

    self.Member("IsMaster") & data.isMaster;
    self.Member("Desc") & data.desc;

    self.Member("DeviceMap") & data.deviceMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ResourceRestructuringData& data)
{
    self.StartObject();

    self.Member("ResourceRestructuringID") & data.resourceRestructuringID;
    self.Member("Name") & data.name;

    int workMode = 0;
    self.Member("WorkMode") & workMode;
    data.workMode = (SystemWorkMode)workMode;

    self.Member("IsMaster") & data.isMaster;
    self.Member("Desc") & data.desc;

    self.Member("DeviceMap") & data.deviceMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ResourceRestructuringData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ResourceRestructuringData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ResourceRestructuringData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ResourceRestructuringData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringDataMap& data)
{
    self.StartObject();
    self.Member("ResourceRestructuringData");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ResourceRestructuringDataMap& data)
{
    self.StartObject();
    self.Member("ResourceRestructuringData");
    std::size_t count = 0;

    self.StartArray(&count);
    ResourceRestructuringData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        data.insert(info.resourceRestructuringID, info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ResourceRestructuringDataMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ResourceRestructuringDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ResourceRestructuringDataMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const ResourceRestructuringDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringDataList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ResourceRestructuringDataList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    ResourceRestructuringData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        data.append(info);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ResourceRestructuringDataList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ResourceRestructuringDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ResourceRestructuringDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ResourceRestructuringDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
