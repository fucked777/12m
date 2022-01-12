#include "TaskReportMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const TaskReportTable& data)
{
    self.StartObject();

    self.Member("uuid") & data.uuid;
    self.Member("createTime") & data.createTime;
    self.Member("preStartTime") & data.preStartTime;
    self.Member("startTime") & data.startTime;
    self.Member("endTime") & data.endTime;
    self.Member("planSerialNumber") & data.planSerialNumber;
    self.Member("taskCode") & data.taskCode;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TaskReportTable& data)
{
    self.StartObject();

    self.Member("uuid") & data.uuid;
    self.Member("createTime") & data.createTime;
    self.Member("preStartTime") & data.preStartTime;
    self.Member("startTime") & data.startTime;
    self.Member("endTime") & data.endTime;
    self.Member("planSerialNumber") & data.planSerialNumber;
    self.Member("taskCode") & data.taskCode;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TaskReportTable& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const TaskReportTable& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TaskReportTable& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TaskReportTable& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const TaskReportTableMap& data)
{
    self.StartObject();
    self.Member("TaskReportTable");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TaskReportTableMap& data)
{
    self.StartObject();
    self.Member("TaskReportTable");
    std::size_t count = 0;

    self.StartArray(&count);
    TaskReportTable taskReport;
    for (std::size_t i = 0; i < count; i++)
    {
        self& taskReport;
        data.insert(taskReport.uuid, taskReport);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TaskReportTableMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const TaskReportTableMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TaskReportTableMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const TaskReportTableMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const TaskReportTableList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, TaskReportTableList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    TaskReportTable taskReport;
    for (std::size_t i = 0; i < count; i++)
    {
        self& taskReport;
        data.append(taskReport);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, TaskReportTableList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const TaskReportTableList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TaskReportTableList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TaskReportTableList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
