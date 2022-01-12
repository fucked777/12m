#include "EphemerisDataSerialize.h"
JsonWriter& operator&(JsonWriter& self, const EphemerisDataData& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");
    self.StartObject();
    self.Member("satellitID") & value.satellitCode;
    self.Member("dateOfEpoch") & value.dateOfEpoch;
    self.Member("timeOfEpoch") & value.timeOfEpoch;
    self.Member("semimajorAxis") & value.semimajorAxis;
    self.Member("eccentricity") & value.eccentricity;
    self.Member("dipAngle") & value.dipAngle;
    self.Member("rightAscension") & value.rightAscension;
    self.Member("argumentOfPerigee") & value.argumentOfPerigee;
    self.Member("meanAnomaly") & value.meanAnomaly;
    self.EndObject();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, EphemerisDataData& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");
    self.StartObject();
    self.Member("satellitID") & value.satellitCode;
    self.Member("dateOfEpoch") & value.dateOfEpoch;
    self.Member("timeOfEpoch") & value.timeOfEpoch;
    self.Member("semimajorAxis") & value.semimajorAxis;
    self.Member("eccentricity") & value.eccentricity;
    self.Member("dipAngle") & value.dipAngle;
    self.Member("rightAscension") & value.rightAscension;
    self.Member("argumentOfPerigee") & value.argumentOfPerigee;
    self.Member("meanAnomaly") & value.meanAnomaly;
    self.EndObject();
    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const EphemerisDataMap& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, EphemerisDataMap& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");
    std::size_t count = 0;

    self.StartArray(&count);
    EphemerisDataData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.insert(info.satellitCode, info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const EphemerisDataList& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, EphemerisDataList& value)
{
    self.StartObject();
    self.Member("EphemerisDataData");
    std::size_t count = 0;

    self.StartArray(&count);
    EphemerisDataData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, EphemerisDataData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const EphemerisDataData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const EphemerisDataData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, EphemerisDataData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, EphemerisDataMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const EphemerisDataMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const EphemerisDataMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, EphemerisDataMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, EphemerisDataList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const EphemerisDataList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const EphemerisDataList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, EphemerisDataList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
