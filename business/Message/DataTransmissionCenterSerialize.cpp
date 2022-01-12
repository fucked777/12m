#include "DataTransmissionCenterSerialize.h"

JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterData& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");
    self.StartObject();
    self.Member("centerName") & value.centerName;
    self.Member("centerIP") & value.centerIP;
    self.Member("centerXS") & value.centerXS;
    self.Member("plaintextDestPortPDXP") & value.plaintextDestPortPDXP;
    self.Member("ciphertextDestPortPDXP") & value.ciphertextDestPortPDXP;
    self.Member("plaintextSrcPortPDXP") & value.plaintextSrcPortPDXP;
    self.Member("ciphertextSrcPortPDXP") & value.ciphertextSrcPortPDXP;
    self.Member("plaintextDestPortFep") & value.plaintextDestPortFep;
    self.Member("ciphertextDestPortFep") & value.ciphertextDestPortFep;
    self.Member("plaintextSrcPortFep") & value.plaintextSrcPortFep;
    self.Member("ciphertextSrcPortFep") & value.ciphertextSrcPortFep;
    self.Member("plaintextDestPortFepRECP") & value.plaintextDestPortFepRECP;
    self.Member("ciphertextDestPortFepRECP") & value.ciphertextDestPortFepRECP;
    self.Member("plaintextSrcPortFepRECP") & value.plaintextSrcPortFepRECP;
    self.Member("ciphertextSrcPortFepRECP") & value.ciphertextSrcPortFepRECP;
    self.Member("desc") & value.desc;
    self.EndObject();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DataTransmissionCenterData& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");
    self.StartObject();
    self.Member("centerName") & value.centerName;
    self.Member("centerIP") & value.centerIP;
    self.Member("centerXS") & value.centerXS;
    self.Member("plaintextDestPortPDXP") & value.plaintextDestPortPDXP;
    self.Member("ciphertextDestPortPDXP") & value.ciphertextDestPortPDXP;
    self.Member("plaintextSrcPortPDXP") & value.plaintextSrcPortPDXP;
    self.Member("ciphertextSrcPortPDXP") & value.ciphertextSrcPortPDXP;
    self.Member("plaintextDestPortFep") & value.plaintextDestPortFep;
    self.Member("ciphertextDestPortFep") & value.ciphertextDestPortFep;
    self.Member("plaintextSrcPortFep") & value.plaintextSrcPortFep;
    self.Member("ciphertextSrcPortFep") & value.ciphertextSrcPortFep;
    self.Member("plaintextDestPortFepRECP") & value.plaintextDestPortFepRECP;
    self.Member("ciphertextDestPortFepRECP") & value.ciphertextDestPortFepRECP;
    self.Member("plaintextSrcPortFepRECP") & value.plaintextSrcPortFepRECP;
    self.Member("ciphertextSrcPortFepRECP") & value.ciphertextSrcPortFepRECP;
    self.Member("desc") & value.desc;
    self.EndObject();
    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterMap& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DataTransmissionCenterMap& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");
    std::size_t count = 0;

    self.StartArray(&count);
    DataTransmissionCenterData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.insert(info.centerName, info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterList& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DataTransmissionCenterList& value)
{
    self.StartObject();
    self.Member("DataTransmissionCenterData");
    std::size_t count = 0;

    self.StartArray(&count);
    DataTransmissionCenterData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DataTransmissionCenterData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const DataTransmissionCenterData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, DataTransmissionCenterData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, DataTransmissionCenterMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const DataTransmissionCenterMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, DataTransmissionCenterMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, DataTransmissionCenterList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const DataTransmissionCenterList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, DataTransmissionCenterList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
