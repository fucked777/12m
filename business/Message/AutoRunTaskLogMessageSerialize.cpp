#include "AutoRunTaskLogMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const AutoTaskLogData& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;                  //任务的唯一ID
    self.Member("taskCode") & data.taskCode;          // 任务代号
    self.Member("serialNumber") & data.serialNumber;  // 计划流水号
    self.Member("createTime") & data.createTime;      // 日志生成时间

    int level = (int)data.level;
    self.Member("level") & level;  // 提示信息 警告信息 错误信息

    self.Member("userID") & data.userID;    // 操作用户ID
    self.Member("context") & data.context;  // 任务日志具体内容

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutoTaskLogData& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;                  //任务的唯一ID
    self.Member("taskCode") & data.taskCode;          // 任务代号
    self.Member("serialNumber") & data.serialNumber;  // 计划流水号
    self.Member("createTime") & data.createTime;      // 日志生成时间

    int level = 0;
    self.Member("level") & level;  // 提示信息 警告信息 错误信息
    data.level = AutoTaskLogLevel(level);

    self.Member("userID") & data.userID;    // 操作用户ID
    self.Member("context") & data.context;  // 任务日志具体内容

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutoTaskLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const AutoTaskLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, AutoTaskLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const AutoTaskLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
JsonWriter& operator&(JsonWriter& self, const AutoTaskLogDataList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }

    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, AutoTaskLogDataList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutoTaskLogData item;
        self& item;
        data << item;
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, AutoTaskLogDataList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const AutoTaskLogDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, AutoTaskLogDataList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const AutoTaskLogDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutoTaskLogDataCondition& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;            //任务的唯一ID
    self.Member("startTime") & data.startTime;  // 任务代号
    self.Member("endTime") & data.endTime;      // 计划流水号

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, AutoTaskLogDataCondition& data)
{
    self.StartObject();
    self.Member("uuid") & data.uuid;            //任务的唯一ID
    self.Member("startTime") & data.startTime;  // 任务代号
    self.Member("endTime") & data.endTime;      // 计划流水号

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, AutoTaskLogDataCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const AutoTaskLogDataCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, AutoTaskLogDataCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const AutoTaskLogDataCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
