#include "DeviceStatusLogMessageSerialize.h"
#include "CustomPacketMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const DeviceStatusRecord& value)
{
    self.StartObject();

    self.Member("id") & value.id;
    self.Member("createTime") & value.createTime;
    self.Member("deviceId") & value.deviceId;
    self.Member("unitId") & value.unitId;
    self.Member("extensionMsg") & value.extensionMsg;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, DeviceStatusRecord& value)
{
    self.StartObject();

    self.Member("id") & value.id;
    self.Member("createTime") & value.createTime;
    self.Member("deviceId") & value.deviceId;
    self.Member("unitId") & value.unitId;
    self.Member("extensionMsg") & value.extensionMsg;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, DeviceStatusRecord& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QByteArray& operator<<(QByteArray& self, const DeviceStatusRecord& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, DeviceStatusRecord& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QString& operator<<(QString& self, const DeviceStatusRecord& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
