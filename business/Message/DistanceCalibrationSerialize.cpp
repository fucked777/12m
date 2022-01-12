#include "DistanceCalibrationSerialize.h"

JsonWriter& operator&(JsonWriter& self, const DistanceCalibrationItem& value)
{
    self.StartObject();
    self.Member("DistanceCalibrationItemData");

    self.StartObject();
    self.Member("id") & value.id;
    self.Member("createTime") & value.createTime;
    self.Member("channelSel") & value.channel;
    self.Member("equipComb") & value.equipComb;
    self.Member("uplinkFreq") & value.uplinkFreq;
    self.Member("downFreq") & value.downFreq;
    self.Member("correctValue") & value.correctValue;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("statisPoint") & value.statisPoint;
    self.Member("signalRot") & value.signalRot;
    self.Member("distZeroMean") & value.distZeroMean;
    self.Member("distZeroVar") & value.distZeroVar;
    self.Member("workMode") & value.workMode;
    self.Member("projectCode") & value.projectCode;
    self.Member("taskCode") & value.taskCode;
    self.Member("dpNum") & value.dpNum;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DistanceCalibrationItem& value)
{
    self.StartObject();
    self.Member("DistanceCalibrationItemData");

    self.StartObject();
    self.Member("id") & value.id;
    self.Member("createTime") & value.createTime;
    self.Member("channelSel") & value.channel;
    self.Member("equipComb") & value.equipComb;
    self.Member("uplinkFreq") & value.uplinkFreq;
    self.Member("downFreq") & value.downFreq;
    self.Member("correctValue") & value.correctValue;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("statisPoint") & value.statisPoint;
    self.Member("signalRot") & value.signalRot;
    self.Member("distZeroMean") & value.distZeroMean;
    self.Member("distZeroVar") & value.distZeroVar;
    self.Member("workMode") & value.workMode;
    self.Member("projectCode") & value.projectCode;
    self.Member("taskCode") & value.taskCode;
    self.Member("dpNum") & value.dpNum;
    self.EndObject();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const DistanceCalibrationItemList& value)
{
    self.StartObject();
    self.Member("DistanceCalibrationItemData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, DistanceCalibrationItemList& value)
{
    self.StartObject();
    self.Member("DistanceCalibrationItemData");
    std::size_t count = 0;

    self.StartArray(&count);
    DistanceCalibrationItem info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const ClearDistanceCalibrationHistry& value)
{
    self.StartObject();
    self.Member("ClearHistryData");

    self.StartObject();
    self.Member("timeInterval") & value.timeInterval;
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ClearDistanceCalibrationHistry& value)
{
    self.StartObject();
    self.Member("ClearHistryData");

    self.StartObject();
    self.Member("timeInterval") & value.timeInterval;
    self.EndObject();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const QueryDistanceCalibrationHistry& value)
{
    self.StartObject();
    self.Member("QueryDistanceCalibrationHistryData");

    self.StartObject();
    self.Member("taskCode") & value.taskCode;
    self.Member("beginTime") & value.beginTime;
    self.Member("endTime") & value.endTime;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, QueryDistanceCalibrationHistry& value)
{
    self.StartObject();
    self.Member("QueryDistanceCalibrationHistryData");

    self.StartObject();
    self.Member("taskCode") & value.taskCode;
    self.Member("beginTime") & value.beginTime;
    self.Member("endTime") & value.endTime;
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, DistanceCalibrationItem& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const DistanceCalibrationItem& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const DistanceCalibrationItem& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, DistanceCalibrationItem& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QByteArray& operator>>(QByteArray& self, DistanceCalibrationItemList& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const DistanceCalibrationItemList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const DistanceCalibrationItemList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, DistanceCalibrationItemList& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QByteArray& operator>>(QByteArray& self, ClearDistanceCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ClearDistanceCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const ClearDistanceCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, ClearDistanceCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QByteArray& operator>>(QByteArray& self, QueryDistanceCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const QueryDistanceCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const QueryDistanceCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, QueryDistanceCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
