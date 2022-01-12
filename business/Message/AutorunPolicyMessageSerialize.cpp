#include "AutorunPolicyMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const AutorunPolicyData& data)
{
    self.StartObject();
    self.Member("AutorunPolicyData");
    self.StartObject();
    self& data.policy;
    self.EndObject();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutorunPolicyData& data)
{
    self.StartObject();
    self.Member("AutorunPolicyData");
    self.StartObject();
    self& data.policy;
    self.EndObject();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutorunPolicyData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const AutorunPolicyData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, AutorunPolicyData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const AutorunPolicyData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
