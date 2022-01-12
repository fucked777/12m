#include "StationResManagementSerialize.h"

JsonWriter& operator&(JsonWriter& self, const StationResManagementData& value)
{
    self.StartObject();
    self.Member("StationResManagementData");
    self.StartObject();
    self.Member("stationIdentifying") & value.stationIdentifying;
    self.Member("stationName") & value.stationName;
    self.Member("ttcUACAddr") & value.ttcUACAddr;
    self.Member("dataTransUACAddr") & value.dataTransUACAddr;
    self.Member("desc") & value.descText;
    self.Member("stationLongitude") & value.stationLongitude;
    self.Member("stationLatitude") & value.stationLatitude;
    self.Member("stationHeight") & value.stationHeight;
    self.Member("isUsable") & value.isUsable;
    self.EndObject();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StationResManagementData& value)
{
    self.StartObject();
    self.Member("StationResManagementData");
    self.StartObject();
    self.Member("stationIdentifying") & value.stationIdentifying;
    self.Member("stationName") & value.stationName;
    self.Member("ttcUACAddr") & value.ttcUACAddr;
    self.Member("dataTransUACAddr") & value.dataTransUACAddr;
    self.Member("desc") & value.descText;
    self.Member("stationLongitude") & value.stationLongitude;
    self.Member("stationLatitude") & value.stationLatitude;
    self.Member("stationHeight") & value.stationHeight;
    self.Member("isUsable") & value.isUsable;
    self.EndObject();
    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const StationResManagementMap& value)
{
    self.StartObject();
    self.Member("StationResManagementData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StationResManagementMap& value)
{
    self.StartObject();
    self.Member("StationResManagementData");
    std::size_t count = 0;

    self.StartArray(&count);
    StationResManagementData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.insert(info.stationIdentifying, info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const StationResManagementList& value)
{
    self.StartObject();
    self.Member("StationResManagementData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StationResManagementList& value)
{
    self.StartObject();
    self.Member("StationResManagementData");
    std::size_t count = 0;

    self.StartArray(&count);
    StationResManagementData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, StationResManagementData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const StationResManagementData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const StationResManagementData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, StationResManagementData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, StationResManagementMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const StationResManagementMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const StationResManagementMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, StationResManagementMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, StationResManagementList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const StationResManagementList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const StationResManagementList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, StationResManagementList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
