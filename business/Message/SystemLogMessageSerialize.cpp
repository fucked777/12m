#include "SystemLogMessageSerialize.h"
#include <QColor>

JsonWriter& operator&(JsonWriter& self, const SystemLogData& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Level") & static_cast<int>(data.level);
    self.Member("Type") & static_cast<int>(data.type);
    self.Member("Context") & data.context;
    self.Member("UserID") & data.userID;
    self.Member("Module") & data.module;
    self.Member("CreateTime") & data.createTime;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, SystemLogData& data)
{
    self.StartObject();

    self.Member("Id") & data.id;

    int level = 0;
    self.Member("Level") & level;
    data.level = LogLevel(level);

    int type = 0;
    self.Member("Type") & type;
    data.type = LogType(type);

    self.Member("Context") & data.context;
    self.Member("UserID") & data.userID;
    self.Member("Module") & data.module;
    self.Member("CreateTime") & data.createTime;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SystemLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const SystemLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, SystemLogData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const SystemLogData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<SystemLogData>& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }

    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QList<SystemLogData>& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        SystemLogData item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QList<SystemLogData>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QList<SystemLogData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QList<SystemLogData>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QList<SystemLogData>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const SystemLogCondition& data)
{
    self.StartObject();

    self.Member("startTime") & data.startTime;
    self.Member("endTime") & data.endTime;
    self.Member("Level") & static_cast<int>(data.level);
    self.Member("currentPage") & data.currentPage;
    self.Member("pageSize") & data.pageSize;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, SystemLogCondition& data)
{
    self.StartObject();

    self.Member("startTime") & data.startTime;
    self.Member("endTime") & data.endTime;

    int level = 0;
    self.Member("Level") & level;
    data.level = LogLevel(level);

    self.Member("currentPage") & data.currentPage;
    self.Member("pageSize") & data.pageSize;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SystemLogCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const SystemLogCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, SystemLogCondition& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const SystemLogCondition& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const SystemLogACK& data)
{
    self.StartObject();

    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("json") & data.dataList;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, SystemLogACK& data)
{
    self.StartObject();

    self.Member("totalCount") & data.totalCount;
    self.Member("currentPage") & data.currentPage;
    self.Member("json") & data.dataList;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SystemLogACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const SystemLogACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, SystemLogACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const SystemLogACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

QString SystemLogHelper::logLevelToString(const LogLevel& level)
{
    switch (level)
    {
    case LogLevel::Info:
    {
        return "提示";
    }
    case LogLevel::Warning:
    {
        return "警告";
    }
    case LogLevel::Error:
    {
        return "错误";
    }
    case LogLevel::SpecificTips:
    {
        return "关键提示";
    }
    }
    return "-";
}
QColor SystemLogHelper::logLevelToColor(const LogLevel& level)
{
    switch (level)
    {
    case LogLevel::Info:
    {
        return QColor(Qt::black);
    }
    case LogLevel::Warning:
    {
        return QColor(229, 153, 0);
    }
    case LogLevel::Error:
    {
        return QColor(231, 35, 35);
    }
    }
    return QColor(Qt::black);
}
QString SystemLogHelper::logTypeString(const LogType& type)
{
    switch (type)
    {
    case LogType::User: return QString("用户操作日志");
    case LogType::System: return QString("系统日志");
    }
    return QString("未知");
}
