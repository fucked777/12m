#include "SatelliteManagementSerialize.h"

#define SM_UNKNOWN_STR      "UNKNOWN"
#define SM_UNKNOWN_DESC_STR "未知"

#define SMSTM_S_STR                   "S"
#define SMSTM_KATELEMETRY_STR         "KATELEMETRY"
#define SMSTM_KADATATRANSMISSION_STR  "KADATATRANSMISSION"
#define SMSTM_SKATELEMETRY_STR        "SKATELEMETRY"
#define SMSTM_SKADATATRANSMISSION_STR "SKADATATRANSMISSION"

#define SMSTM_S_DESC_STR                   "S"
#define SMSTM_KATELEMETRY_DESC_STR         "Ka遥测"
#define SMSTM_KADATATRANSMISSION_DESC_STR  "Ka数传"
#define SMSTM_SKATELEMETRY_DESC_STR        "S+Ka遥测"
#define SMSTM_SKADATATRANSMISSION_DESC_STR "S+Ka数传"

#define SMSTM_HIGHORBIT_STR "HIGHORBIT"
#define SMSTM_LOWORBIT_STR  "LOWORBIT"

#define SMSTM_HIGHORBIT_DESC_STR "高轨"
#define SMSTM_LOWORBIT_DESC_STR  "低轨"

JsonWriter& operator&(JsonWriter& self, const SatelliteManagementParamItem& entity)
{
    self.StartObject();
    for (auto key : entity.m_itemMap.keys())
    {
        auto _value = entity.m_itemMap.value(key);
        self.Member(key) & _value;
    }
    self.EndObject();
    return self;
}

JsonReader& operator&(JsonReader& self, SatelliteManagementParamItem& entity)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QString tempKey = key;
        QVariant temp;
        self.Member(tempKey) & temp;
        entity.m_itemMap[tempKey] = temp;
    }
    self.EndObject();

    return self;
}

JsonWriter& operator&(JsonWriter& self, const QList<SatelliteManagementParamItem>& list)
{
    self.StartObject();

    auto tempSize = list.size();
    self.Member("size") & tempSize;
    self.Member("SatelliteManagementParamItemList");
    {
        self.StartArray();
        for (int i = 0; i < list.size(); i++)
            self& list[i];
        self.EndArray();
    }
    self.EndObject();
    return self;
}

JsonReader& operator&(JsonReader& self, QList<SatelliteManagementParamItem>& list)
{
    self.StartObject();
    std::size_t size = 0;
    self.Member("size") & size;
    self.Member("SatelliteManagementParamItemList");
    {
        self.StartArray(&size);

        for (int i = 0; i < static_cast<int>(size); i++)
        {
            SatelliteManagementParamItem temp;
            self& temp;
            list.append(temp);
        }

        self.EndArray();
    }

    self.EndObject();
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, SatelliteManagementParamItem>& s)
{
    self.StartObject();

    self.Member("SatelliteManagementParamItemMap");

    self.StartObject();
    for (auto key : s.keys())
    {
        auto value = s.value(key);
        self.Member(QString::number(key)) & value;
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, SatelliteManagementParamItem>& s)
{
    self.StartObject();

    self.Member("SatelliteManagementParamItemMap");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QString tempKey = key;
        SatelliteManagementParamItem temp;
        self.Member(tempKey) & temp;
        s[tempKey.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& s)
{
    self.StartObject();

    self.Member("SatelliteManagementParamItemMap");

    self.StartObject();
    for (auto key : s.keys())
    {
        auto value = s.value(key);
        self.Member(QString::number(key)) & value;
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& s)
{
    self.StartObject();

    self.Member("SatelliteManagementParamItemMap");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        QMap<int, SatelliteManagementParamItem> temp;
        self.Member(key) & temp;
        s[(SystemWorkMode)key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SatelliteManagementParamItem& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator<<(QByteArray& self, const SatelliteManagementParamItem& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const SatelliteManagementParamItem& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, SatelliteManagementParamItem& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator>>(QByteArray& self, QList<SatelliteManagementParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator<<(QByteArray& self, const QList<SatelliteManagementParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const QList<SatelliteManagementParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, QList<SatelliteManagementParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator>>(QByteArray& self, QMap<int, SatelliteManagementParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, SatelliteManagementParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const QMap<int, SatelliteManagementParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}

QString& operator>>(QString& self, QMap<int, SatelliteManagementParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}

QString& operator>>(QString& self, QMap<SystemWorkMode, QMap<int, SatelliteManagementParamItem>>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

////////////////////////////////
JsonWriter& operator&(JsonWriter& self, const SatelliteManagementData& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");
    self.StartObject();
    self.Member("satelliteName") & entity.m_satelliteName;
    self.Member("satelliteTaskCode") & entity.m_satelliteCode;
    self.Member("satelliteIdentification") & entity.m_satelliteIdentification;
    self.Member("stationID") & entity.m_stationID;
    self.Member("taskCenterID") & entity.m_taskCenterID;
    self.Member("digitalCenterID") & entity.m_digitalCenterID;
    self.Member("railType") & entity.m_railType;
    self.Member("trackingMode") & entity.m_trackingMode;
    self.Member("sPhasing") & entity.m_sPhasing;
    self.Member("kaPhasing") & entity.m_kaPhasing;
    self.Member("workmode") & entity.m_workMode;
    self.Member("m_isCarrierType") & entity.m_isCarrierType;

    self.Member("modeParamMap");

    self.StartObject();
    for (auto workMode : entity.m_workModeParamMap.keys())
    {
        auto _value = entity.m_workModeParamMap.value(workMode);
        self.Member(QString::number((int)workMode)) & _value;
    }
    self.EndObject();

    self.Member("Desc") & entity.m_desc;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, SatelliteManagementData& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");
    self.StartObject();
    self.Member("satelliteName") & entity.m_satelliteName;
    self.Member("satelliteTaskCode") & entity.m_satelliteCode;
    self.Member("satelliteIdentification") & entity.m_satelliteIdentification;
    self.Member("stationID") & entity.m_stationID;
    self.Member("taskCenterID") & entity.m_taskCenterID;
    self.Member("digitalCenterID") & entity.m_digitalCenterID;
    self.Member("railType") & entity.m_railType;
    self.Member("trackingMode") & entity.m_trackingMode;
    self.Member("sPhasing") & entity.m_sPhasing;
    self.Member("kaPhasing") & entity.m_kaPhasing;
    self.Member("workmode") & entity.m_workMode;
    self.Member("m_isCarrierType") & entity.m_isCarrierType;

    self.Member("modeParamMap");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        SatelliteManagementDpParamItem temp;
        self.Member(key) & temp;
        entity.m_workModeParamMap[(SystemWorkMode)key.toUInt()] = temp;
    }
    self.EndObject();
    self.Member("Desc") & entity.m_desc;
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QList<SatelliteManagementData>& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        SatelliteManagementData info;
        self& info;
        entity.append(info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const QList<SatelliteManagementData>& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");

    self.StartArray();
    for (auto& item : entity)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonWriter& operator&(JsonWriter& self, const QMap<QString, SatelliteManagementData>& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");

    self.StartArray();
    for (auto& item : entity)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<QString, SatelliteManagementData>& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementData");
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        SatelliteManagementData info;
        self& info;
        entity.insert(info.m_satelliteCode, info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SatelliteManagementData& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator<<(QByteArray& self, const SatelliteManagementData& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const SatelliteManagementData& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, SatelliteManagementData& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator>>(QByteArray& self, QList<SatelliteManagementData>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator<<(QByteArray& self, const QList<SatelliteManagementData>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const QList<SatelliteManagementData>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, QList<SatelliteManagementData>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator>>(QByteArray& self, QMap<QString, SatelliteManagementData>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<QString, SatelliteManagementData>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const QMap<QString, SatelliteManagementData>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}

QString& operator>>(QString& self, QMap<QString, SatelliteManagementData>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

////////////////////////////////
JsonWriter& operator&(JsonWriter& self, const SatelliteManagementDpParamItem& entity)
{
    self.StartObject();
    self.Member("defalutNum") & entity.defalutNum;
    self.Member("m_dpInfoMap");
    self.StartObject();
    for (auto workMode : entity.m_dpInfoMap.keys())
    {
        auto _value = entity.m_dpInfoMap.value(workMode);
        self.Member(QString::number((int)workMode)) & _value;
    }
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, SatelliteManagementDpParamItem& entity)
{
    self.StartObject();
    self.Member("defalutNum") & entity.defalutNum;
    self.Member("m_dpInfoMap");
    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        SatelliteManagementParamItem temp;
        self.Member(key) & temp;
        entity.m_dpInfoMap[(SystemWorkMode)key.toUInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    self.StartObject();

    self.Member("SatelliteManagementDpParamItem");
    self.StartObject();
    for (auto key : entity.keys())
    {
        auto value = entity.value(key);
        self.Member(QString::number(key)) & value;
    }
    self.EndObject();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    self.StartObject();
    self.Member("SatelliteManagementDpParamItem");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        SatelliteManagementDpParamItem temp;
        self.Member(key) & temp;
        entity[(SystemWorkMode)key.toInt()] = temp;
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, SatelliteManagementDpParamItem& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
QByteArray& operator<<(QByteArray& self, const SatelliteManagementDpParamItem& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const SatelliteManagementDpParamItem& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, SatelliteManagementDpParamItem& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator<<(QString& self, const QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    JsonWriter writer;
    writer& entity;
    self.append(writer.GetQString());
    return self;
}

QString& operator>>(QString& self, QMap<SystemWorkMode, SatelliteManagementDpParamItem>& entity)
{
    JsonReader reader(self);
    reader& entity;

    return self;
}
