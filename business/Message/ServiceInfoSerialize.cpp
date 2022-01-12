#include "ServiceInfoSerialize.h"
#include <QList>
#include <QObject>

JsonWriter& operator&(JsonWriter& self, const ServiceInfo& value)
{
    self.StartObject();
    self.Member("ServiceInfo");

    self.StartObject();
    self.Member("startTime") & value.startTime;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ServiceInfo& value)
{
    self.StartObject();
    self.Member("ServiceInfo");

    self.StartObject();
    self.Member("startTime") & value.startTime;
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ServiceInfo& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const ServiceInfo& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const ServiceInfo& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, ServiceInfo& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
