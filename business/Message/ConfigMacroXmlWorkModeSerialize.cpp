#include "ConfigMacroXmlWorkModeSerialize.h"

JsonWriter& operator&(JsonWriter& self, const EnumEntry& data)
{
    self.StartObject();

    self.Member("Desc") & data.desc;
    self.Member("Value") & data.value;

    int intDeviceId = 0;
    data.deviceId >> intDeviceId;
    self.Member("DeviceId") & intDeviceId;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, EnumEntry& data)
{
    self.StartObject();

    self.Member("Desc") & data.desc;
    self.Member("Value") & data.value;

    int intDeviceId = 0;
    self.Member("DeviceId") & intDeviceId;
    data.deviceId << intDeviceId;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, EnumEntry& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const EnumEntry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, EnumEntry& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const EnumEntry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const EnumEntryList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, EnumEntryList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        EnumEntry item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, EnumEntryList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const EnumEntryList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, EnumEntryList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const EnumEntryList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const Item& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Desc") & data.desc;
    self.Member("IsManualControl") & data.isManualControl;
    self.Member("Value") & data.value;
    self.Member("startBit") & data.startBit;  // 设备组合号里面的开始位
    self.Member("endBit") & data.endBit;      // 设备组合号里面的结束位
    self.Member("offset") & data.offset;      // 设备组合号里面的偏移量
    self.Member("linkType") & data.linkType;  // 设备组合号类设备上下行区分
    self.Member("Enums") & data.enums;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, Item& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Desc") & data.desc;
    self.Member("IsManualControl") & data.isManualControl;
    self.Member("Value") & data.value;
    self.Member("startBit") & data.startBit;  // 设备组合号里面的开始位
    self.Member("endBit") & data.endBit;      // 设备组合号里面的结束位
    self.Member("offset") & data.offset;      // 设备组合号里面的偏移量
    self.Member("linkType") & data.linkType;  // 设备组合号类设备上下行区分
    self.Member("Enums") & data.enums;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, Item& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const Item& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, Item& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const Item& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ItemList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ItemList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        Item item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ItemList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkMode& data)
{
    self.StartObject();

    self.Member("WorkMode") & static_cast<int>(data.workMode);
    self.Member("Desc") & data.desc;

    self.Member("Items") & data.items;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ConfigMacroWorkMode& data)
{
    self.StartObject();

    int worMode = 0;
    self.Member("WorkMode") & worMode;
    data.workMode = (SystemWorkMode)worMode;

    self.Member("Desc") & data.desc;

    self.Member("Items") & data.items;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ConfigMacroWorkMode& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkMode& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ConfigMacroWorkMode& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const ConfigMacroWorkMode& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkModeList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ConfigMacroWorkModeList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroWorkMode item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ConfigMacroWorkModeList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkModeList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ConfigMacroWorkModeList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ConfigMacroWorkModeList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroWorkModeMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, ConfigMacroWorkModeMap& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroWorkMode item;
        self& item;
        data.insert(item.workMode, item);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, ConfigMacroWorkModeMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const ConfigMacroWorkModeMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ConfigMacroWorkModeMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ConfigMacroWorkModeMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
