#include "LinkConfigMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const Command& data)
{
    self.StartObject();

    self.Member("CmdType") & static_cast<int>(data.cmdType);
    self.Member("DeviceId") & data.deviceId;
    self.Member("DeviceName") & data.deviceName;
    self.Member("ModeId") & data.modeId;
    self.Member("CmdId") & data.cmdId;
    self.Member("OperatorInfo") & data.operatorInfo;

    self.Member("ParamMap");
    self.StartObject();
    for (auto key : data.paramMap.keys())
    {
        auto value = data.paramMap.value(key);
        self.Member(key) & value.toString();
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, Command& data)
{
    self.StartObject();

    int cmdType = 0;
    self.Member("CmdType") & cmdType;
    data.cmdType = DevMsgType(cmdType);

    self.Member("DeviceId") & data.deviceId;
    self.Member("DeviceName") & data.deviceName;
    self.Member("ModeId") & data.modeId;
    self.Member("CmdId") & data.cmdId;
    self.Member("OperatorInfo") & data.operatorInfo;

    self.Member("ParamMap");
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QString tempVal;
        self.Member(key) & tempVal;
        data.paramMap[key] = tempVal;
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, Command& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const Command& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, Command& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const Command& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<Command>& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QList<Command>& data)
{
    std::size_t count = 0;
    self.StartArray(&count);

    for (std::size_t i = 0; i < count; i++)
    {
        Command cmd;
        self& cmd;
        data << cmd;
    }
    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QList<Command>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QList<Command>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QList<Command>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QList<Command>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QSet<QSet<QString>>& data)
{
    self.StartArray();
    for (const auto& itemSet : data.toList())
    {
        self& itemSet;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QSet<QSet<QString>>& data)
{
    std::size_t count1 = 0;
    self.StartArray(&count1);

    for (std::size_t i = 0; i < count1; ++i)
    {
        QSet<QString> set;
        self& set;
        data << set;
    }
    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QSet<QSet<QString>>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QSet<QSet<QString>>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QSet<QSet<QString>>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QSet<QSet<QString>>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const CommandGroup& data)
{
    self.StartObject();

    self.Member("ConditionSet") & data.conditionSet;
    self.Member("Commands") & data.commands;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, CommandGroup& data)
{
    self.StartObject();

    self.Member("ConditionSet") & data.conditionSet;
    self.Member("Commands") & data.commands;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, CommandGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const CommandGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, CommandGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const CommandGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const LinkMode& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Name") & data.name;

    self.Member("CmdGroups") & data.cmdGroups;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, LinkMode& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Name") & data.name;

    self.Member("CmdGroups") & data.cmdGroups;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, LinkMode& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const LinkMode& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, LinkMode& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const LinkMode& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, LinkMode>& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number(key)) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, LinkMode>& data)
{
    self.StartObject();

    auto keys = self.Members();
    for (auto& key : keys)
    {
        LinkMode mode;
        self.Member(key) & mode;
        data[key.toInt()] = mode;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, LinkMode>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, LinkMode>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, LinkMode>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QMap<int, LinkMode>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const LinkConfig& data)
{
    self.StartObject();

    self.Member("LinkType") & static_cast<int>(data.linkType);
    self.Member("Name") & data.name;

    self.Member("ModeMap") & data.modeMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, LinkConfig& data)
{
    self.StartObject();

    self.Member("LinkType") & data.linkType;
    self.Member("Name") & data.name;

    self.Member("ModeMap") & data.modeMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, LinkConfig& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const LinkConfig& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, LinkConfig& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const LinkConfig& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const LinkConfigMap& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number(static_cast<int>(key))) & value;
    }

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, LinkConfigMap& data)
{
    self.StartObject();

    auto keys = self.Members();
    for (auto& key : keys)
    {
        LinkConfig linkConfig;
        self.Member(key) & linkConfig;

        data[(LinkType)key.toInt()] = linkConfig;
    }

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, LinkConfigMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const LinkConfigMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, LinkConfigMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const LinkConfigMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<CommandGroup>& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QList<CommandGroup>& data)
{
    std::size_t count = 0;
    self.StartArray(&count);

    for (std::size_t i = 0; i < count; i++)
    {
        CommandGroup cmd;
        self& cmd;
        data << cmd;
    }
    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QList<CommandGroup>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QList<CommandGroup>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QList<CommandGroup>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QList<CommandGroup>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QSet<QString>& data)
{
    self.StartArray();
    for (const auto& item : data.toList())
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, QSet<QString>& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; ++i)
    {
        QString value;
        self& value;
        data << value;
    }
    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, QSet<QString>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QSet<QString>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QSet<QString>& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const QSet<QString>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
