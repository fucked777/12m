#include "TimeMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const TimeData& data)
{
    self.StartObject();

    self.Member("UsedTimeType") & static_cast<int>(data.usedTimeType);

    self.Member("BCodeTime") & data.bCodeTime;

    self.Member("NtpTime") & data.ntpTime;
    self.Member("NtpServerAddr") & data.ntpServerAddr.toString();

    self.Member("ServerSystemTime") & data.serverSystemTime;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TimeData& data)
{
    self.StartObject();

    int usedTimeType = 0;
    self.Member("UsedTimeType") & usedTimeType;
    data.usedTimeType = (TimeType)usedTimeType;

    self.Member("BCodeTime") & data.bCodeTime;
    self.Member("NtpTime") & data.ntpTime;
    QString addr;
    self.Member("NtpServerAddr") & addr;
    data.ntpServerAddr = QHostAddress(addr);

    self.Member("ServerSystemTime") & data.serverSystemTime;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TimeData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const TimeData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TimeData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TimeData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const TimeConfigInfo& data)
{
    self.StartObject();

    self.Member("UseTtimeType") & data.usedTimeType;

    self.Member("NtpMasterServerAddr") & data.ntpMasterServerAddr.toString();
    self.Member("NtpSlaveServerAddr") & data.ntpSlaveServerAddr.toString();

    self.Member("MaxJumpInterval") & data.maxJumpInterval;
    self.Member("LongTimeNotUpdatedInterval") & data.longTimeNotUpdatedInterval;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TimeConfigInfo& data)
{
    self.StartObject();

    self.Member("UseTtimeType") & data.usedTimeType;

    QString addr;
    self.Member("NtpMasterServerAddr") & addr;
    data.ntpMasterServerAddr = QHostAddress(addr);

    self.Member("NtpSlaveServerAddr") & addr;
    data.ntpSlaveServerAddr = QHostAddress(addr);

    self.Member("MaxJumpInterval") & data.maxJumpInterval;
    self.Member("LongTimeNotUpdatedInterval") & data.longTimeNotUpdatedInterval;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TimeConfigInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const TimeConfigInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TimeConfigInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TimeConfigInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
