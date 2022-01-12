#include "HeartbeatMessageSerialize.h"
#include <QList>
#include <QObject>

JsonWriter& operator&(JsonWriter& self, const HeartbeatMessage& value)
{
    self.StartObject();

    self.Member("isUsed") & value.isUsed;
    self.Member("status") & value.status;
    self.Member("serverID") & value.serverID;
    self.Member("currentMasterIP") & value.currentMasterIP;

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, HeartbeatMessage& value)
{
    self.StartObject();

    self.Member("isUsed") & value.isUsed;
    self.Member("status") & value.status;
    self.Member("serverID") & value.serverID;
    self.Member("currentMasterIP") & value.currentMasterIP;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, HeartbeatMessage& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const HeartbeatMessage& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const HeartbeatMessage& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, HeartbeatMessage& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
