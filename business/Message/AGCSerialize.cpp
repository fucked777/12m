#include "AGCSerialize.h"
#include <QByteArray>
#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QVariant>

JsonWriter& operator&(JsonWriter& self, const AGCResult& data)
{
    self.StartObject();
    // self.Member("ycAGC") & data.ycAGC;
    // self.Member("gzAGC") & data.gzAGC;
    self.Member("agc") & data.agc;
    self.Member("sbf") & data.sbf;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AGCResult& data)
{
    self.StartObject();
    // self.Member("ycAGC") & data.ycAGC;
    // self.Member("gzAGC") & data.gzAGC;
    self.Member("agc") & data.agc;
    self.Member("sbf") & data.sbf;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AGCResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AGCResultList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, AGCResultList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    AGCResult agcResult;
    for (std::size_t i = 0; i < count; i++)
    {
        self& agcResult;
        data.append(agcResult);
    }

    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, AGCResultList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCResultList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCResultList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCResultList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationItem& data)
{
    self.StartObject();
    self.Member("itemGUID") & data.itemGUID;
    self.Member("taskCode") & data.taskCode;               /* 任务代号 6B */
    self.Member("createTime") & data.createTime;           /* 创建时间 */
    self.Member("projectCode") & data.projectCode;         /* 项目代号 6B */
    self.Member("taskID") & data.taskID;                   /* 卫星标识\卫星标识 */
    self.Member("workMode") & data.workMode;               /* 工作模式 */
    self.Member("dotDrequencyNum") & data.dotDrequencyNum; /* 点频 */
    self.Member("equipComb") & data.equipComb;             /* 设备组合号 */
    self.Member("result") & data.result;

    // self.Member("downFreq") & data.downFreq;               /* 下行频率[MHz] */
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AGCCalibrationItem& data)
{
    self.StartObject();
    self.Member("itemGUID") & data.itemGUID;
    self.Member("taskCode") & data.taskCode;               /* 任务代号 6B */
    self.Member("createTime") & data.createTime;           /* 创建时间 */
    self.Member("projectCode") & data.projectCode;         /* 项目代号 6B */
    self.Member("taskID") & data.taskID;                   /* 卫星标识\卫星标识 */
    self.Member("workMode") & data.workMode;               /* 工作模式 */
    self.Member("dotDrequencyNum") & data.dotDrequencyNum; /* 点频 */
    self.Member("equipComb") & data.equipComb;             /* 设备组合号 */
    self.Member("result") & data.result;

    // self.Member("downFreq") & data.downFreq; /* 下行频率[MHz] */
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AGCCalibrationItem& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCCalibrationItem& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCCalibrationItem& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCCalibrationItem& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationItemList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, AGCCalibrationItemList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    AGCCalibrationItem agcResult;
    for (std::size_t i = 0; i < count; i++)
    {
        self& agcResult;
        data.append(agcResult);
    }

    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, AGCCalibrationItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCCalibrationItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCCalibrationItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCCalibrationItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AGCResultQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginTime;
    self.Member("endTime") & data.endTime;
    self.Member("taskCode") & data.taskCode;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AGCResultQuery& data)
{
    self.StartObject();
    self.Member("beginTime") & data.beginTime;
    self.Member("endTime") & data.endTime;
    self.Member("taskCode") & data.taskCode;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AGCResultQuery& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCResultQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCResultQuery& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCResultQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ClearAGCCalibrationHistry& data)
{
    self.StartObject();
    self.Member("timeInterval") & data.timeInterval;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ClearAGCCalibrationHistry& data)
{
    self.StartObject();
    self.Member("timeInterval") & data.timeInterval;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ClearAGCCalibrationHistry& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ClearAGCCalibrationHistry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ClearAGCCalibrationHistry& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ClearAGCCalibrationHistry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationProcessACK& data)
{
    self.StartObject();
    self.Member("result") & data.result;
    self.Member("msg") & data.msg;
    self.Member("value") & data.value;
    if (data.result == AGCCalibrationStatus::End)
    {
        self.Member("item") & data.item;
    }

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AGCCalibrationProcessACK& data)
{
    self.StartObject();
    self.Member("result") & data.result;
    self.Member("msg") & data.msg;
    self.Member("value") & data.value;
    if (data.result == AGCCalibrationStatus::End)
    {
        self.Member("item") & data.item;
    }
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AGCCalibrationProcessACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCCalibrationProcessACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCCalibrationProcessACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCCalibrationProcessACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

#if 0
JsonWriter& operator&(JsonWriter& self, const AGCSendToBS& data)
{
    int deviceID = data.deviceID.toInt();
    self.StartObject();
    self.Member("deviceID") & deviceID;
    self.Member("item") & data.item;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AGCSendToBS& data)
{
    int deviceID = 0;
    self.StartObject();
    self.Member("deviceID") & deviceID;
    data.deviceID << deviceID;

    self.Member("item") & data.item;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AGCSendToBS& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AGCSendToBS& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AGCSendToBS& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AGCSendToBS& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
#endif
