#include "ConfigMacroMessageSerialize.h"
#include <QSet>

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmd& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Desc") & data.desc;
    self.Member("Value") & data.value.toString();
    self.Member("startBit") & data.startBit;
    self.Member("endBit") & data.endBit;
    self.Member("offset") & data.offset;
    self.Member("linkType") & data.linkType;

    int deviceId = 0;
    data.deviceID >> deviceId;
    self.Member("DeviceID") & deviceId;

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ConfigMacroCmd& data)
{
    self.StartObject();

    self.Member("Id") & data.id;
    self.Member("Desc") & data.desc;
    self.Member("Value") & data.value;
    self.Member("startBit") & data.startBit;
    self.Member("endBit") & data.endBit;
    self.Member("offset") & data.offset;
    self.Member("linkType") & data.linkType;

    int deviceId = 0;
    self.Member("DeviceID") & deviceId;
    data.deviceID = DeviceID(deviceId);

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroCmd& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmd& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroCmd& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroCmd& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroCmdList& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroCmd info;
        self& info;
        data << info;
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroCmdList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroCmdList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroCmdMap& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroCmd info;
        self& info;
        data.insert(info.id, info);
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroCmdMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroCmdMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroModeItemMap& data)
{
    self.StartObject();
    self.Member("WorkMode") & data.workMode;

    self.Member("ModeData");
    self.StartArray();
    for (auto& item : data.configMacroCmdMap)
    {
        self& item;
    }
    self.EndArray();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ConfigMacroModeItemMap& data)
{
    self.StartObject();
    self.Member("WorkMode") & data.workMode;

    self.Member("ModeData");
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroCmd item;
        self& item;
        data.configMacroCmdMap.insert(item.id, item);
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroModeItemMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroModeItemMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroModeItemMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QString& operator<<(QString& self, const ConfigMacroModeItemMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroModeItemList& data)
{
    self.StartObject();
    self.Member("WorkMode") & data.workMode;

    self.Member("ModeData");
    self.StartArray();
    for (auto& item : data.configMacroCmdList)
    {
        self& item;
    }
    self.EndArray();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ConfigMacroModeItemList& data)
{
    self.StartObject();
    self.Member("WorkMode") & data.workMode;

    self.Member("ModeData");
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroCmd item;
        self& item;
        data.configMacroCmdList.append(item);
    }

    self.EndArray();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroModeItemList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroModeItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroModeItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroModeItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdModeMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroCmdModeMap& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroModeItemMap item;
        self& item;
        data.insert(item.workMode, item);
    }

    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdModeMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdModeMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroCmdModeMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroCmdModeMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroCmdModeList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroCmdModeList& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroModeItemList item;
        self& item;
        data.append(item);
    }

    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroCmdModeList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroCmdModeList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroCmdModeList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroCmdModeList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroData& data)
{
    self.StartObject();

    self.Member("ConfigMacroID") & data.configMacroID;
    self.Member("ConfigMacroName") & data.configMacroName;
    self.Member("WorkModeDesc") & data.workModeDesc;
    self.Member("StationID") & data.stationID;
    self.Member("IsMaster") & data.isMaster;
    self.Member("Desc") & data.desc;

    self.Member("ConfigMacroCmdMap") & data.configMacroCmdModeMap;

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ConfigMacroData& data)
{
    self.StartObject();

    self.Member("ConfigMacroID") & data.configMacroID;
    self.Member("ConfigMacroName") & data.configMacroName;
    self.Member("WorkModeDesc") & data.workModeDesc;
    self.Member("StationID") & data.stationID;
    self.Member("IsMaster") & data.isMaster;
    self.Member("Desc") & data.desc;

    self.Member("ConfigMacroCmdMap") & data.configMacroCmdModeMap;

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroData& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroData& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroDataMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroDataMap& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroData info;
        self& info;
        data.insert(info.configMacroID, info);
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroDataMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroDataMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}
QString& operator<<(QString& self, const ConfigMacroDataMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ConfigMacroDataList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, ConfigMacroDataList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        ConfigMacroData info;
        self& info;
        data.append(info);
    }

    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, ConfigMacroDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ConfigMacroDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ConfigMacroDataList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ConfigMacroDataList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

QString ConfigMacroMessageHelper::createSearchName(const QSet<SystemWorkMode>& modeSet, MasterSlave masterSlave)
{
    QStringList modeStrList;
    for (auto& item : modeSet)
    {
        modeStrList << SystemWorkModeHelper::systemWorkModeToString(item);
    }
    return QString("%1_%2").arg(modeStrList.join("+")).arg(static_cast<int>(masterSlave));
}
QString ConfigMacroMessageHelper::createSearchName(const SystemWorkMode& mode, MasterSlave masterSlave)
{
    return QString("%1_%2").arg(SystemWorkModeHelper::systemWorkModeToString(mode)).arg(static_cast<int>(masterSlave));
}
