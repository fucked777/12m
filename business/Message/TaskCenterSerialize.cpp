#include "TaskCenterSerialize.h"
#include <QList>
#include <QObject>

#define TC_UNKNOWN_STR      "UNKNOWN"
#define TC_UNKNOWN_DESC_STR "未知"

#define TCDC_MACC_STR      "MACC"
#define TCDC_MACC_DESC_STR "测控中心"

#define TCFF_PDXP_STR      "PDXP"
#define TCFF_PDXP_DESC_STR "PDXP"

QString TCConvert::tcdcToStr(TCDirectionType type)
{
    switch (type)
    {
    case TCDirectionType::Unknown:
    {
        break;
    }
    case TCDirectionType::MACC:
    {
        return TCDC_MACC_STR;
    }
    }
    return TC_UNKNOWN_STR;
}
TCDirectionType TCConvert::tcdcFromStr(const QString& raw)
{
    auto type = raw.toUpper();
    if (type == TCDC_MACC_STR)
    {
        return TCDirectionType::MACC;
    }
    return TCDirectionType::Unknown;
}
QString TCConvert::tcdcToDesStr(TCDirectionType type)
{
    switch (type)
    {
    case TCDirectionType::Unknown:
    {
        break;
    }
    case TCDirectionType::MACC:
    {
        return TCDC_MACC_DESC_STR;
    }
    }
    return TC_UNKNOWN_DESC_STR;
}
QList<TCDirectionType> TCConvert::tcdcList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<TCDirectionType> tcList{
            TCDirectionType::Unknown,
            TCDirectionType::MACC,
        };
        return tcList;
    }
    static QList<TCDirectionType> tcList{
        TCDirectionType::MACC,
    };
    return tcList;
}
static QList<EnumInfo<TCDirectionType>> tcdcListInfoImpl()
{
    using IT = EnumInfo<TCDirectionType>;
    QList<EnumInfo<TCDirectionType>> ret;

    ret.append(IT{ TCDirectionType::MACC, TCDC_MACC_STR, TCDC_MACC_DESC_STR });
    return ret;
}
static QList<EnumInfo<TCDirectionType>> tcdcListInfoImplUnknown()
{
    using IT = EnumInfo<TCDirectionType>;
    QList<EnumInfo<TCDirectionType>> ret;

    ret.append(IT{ TCDirectionType::Unknown, TC_UNKNOWN_STR, TC_UNKNOWN_DESC_STR });
    ret.append(tcdcListInfoImpl());
    return ret;
}

QList<EnumInfo<TCDirectionType>> TCConvert::tcdcListInfo(bool hasUnknown)
{
    if (hasUnknown)
    {
        static auto tempList = tcdcListInfoImplUnknown();
        return tempList;
    }

    static auto tempList = tcdcListInfoImpl();
    return tempList;
}

QString TCConvert::tcffToStr(TCFrameFormat type)
{
    switch (type)
    {
    case TCFrameFormat::Unknown:
    {
        break;
    }
    case TCFrameFormat::PDXP:
    {
        return TCFF_PDXP_STR;
    }
    }
    return TC_UNKNOWN_STR;
}
TCFrameFormat TCConvert::tcffFromStr(const QString& raw)
{
    auto type = raw.toUpper();
    if (type == TCFF_PDXP_STR)
    {
        return TCFrameFormat::PDXP;
    }
    return TCFrameFormat::Unknown;
}
QString TCConvert::tcffToDesStr(TCFrameFormat type)
{
    switch (type)
    {
    case TCFrameFormat::Unknown:
    {
        break;
    }
    case TCFrameFormat::PDXP:
    {
        return TCFF_PDXP_DESC_STR;
    }
    }
    return TC_UNKNOWN_DESC_STR;
}
QList<TCFrameFormat> TCConvert::tcffList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<TCFrameFormat> tcList{
            TCFrameFormat::Unknown,
            TCFrameFormat::PDXP,
        };
        return tcList;
    }
    static QList<TCFrameFormat> tcList{
        TCFrameFormat::PDXP,
    };
    return tcList;
}
static QList<EnumInfo<TCFrameFormat>> tcffListInfoImpl()
{
    using IT = EnumInfo<TCFrameFormat>;
    QList<EnumInfo<TCFrameFormat>> ret;

    ret.append(IT{ TCFrameFormat::PDXP, TCFF_PDXP_STR, TCFF_PDXP_STR });
    return ret;
}
static QList<EnumInfo<TCFrameFormat>> tcffListInfoImplUnknown()
{
    using IT = EnumInfo<TCFrameFormat>;
    QList<EnumInfo<TCFrameFormat>> ret;

    ret.append(IT{ TCFrameFormat::Unknown, TC_UNKNOWN_STR, TC_UNKNOWN_DESC_STR });

    ret.append(tcffListInfoImpl());
    return ret;
}
QList<EnumInfo<TCFrameFormat>> TCConvert::tcffListInfo(bool hasUnknown)
{
    if (hasUnknown)
    {
        static auto tempList = tcffListInfoImplUnknown();
        return tempList;
    }

    static auto tempList = tcffListInfoImpl();
    return tempList;
}

JsonWriter& operator&(JsonWriter& self, const TaskCenterData& value)
{
    self.StartObject();
    self.Member("TaskCenterData");
    self.StartObject();
    self.Member("centerName") & value.centerName;
    self.Member("centerCode") & value.centerCode;
    self.Member("frameFormat") & TCConvert::tcffToStr(value.frameFormat);
    self.Member("direType") & TCConvert::tcdcToStr(value.direType);
    self.Member("isUsedLinkMonitor") & value.isUsedLinkMonitor;
    self.Member("isUsed") & value.isUsed;
    self.Member("uacAddr") & value.uacAddr;
    self.EndObject();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, TaskCenterData& value)
{
    self.StartObject();
    self.Member("TaskCenterData");
    self.StartObject();

    self.Member("centerName") & value.centerName;
    self.Member("centerCode") & value.centerCode;
    QString tempStr;
    self.Member("frameFormat") & tempStr;
    value.frameFormat = TCConvert::tcffFromStr(tempStr);
    self.Member("direType") & tempStr;
    value.direType = TCConvert::tcdcFromStr(tempStr);
    self.Member("isUsedLinkMonitor") & value.isUsedLinkMonitor;
    self.Member("isUsed") & value.isUsed;
    self.Member("uacAddr") & value.uacAddr;
    self.EndObject();
    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const TaskCenterMap& value)
{
    self.StartObject();
    self.Member("TaskCenterData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, TaskCenterMap& value)
{
    self.StartObject();
    self.Member("TaskCenterData");
    std::size_t count = 0;

    self.StartArray(&count);
    TaskCenterData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.insert(info.uacAddr, info);
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const TaskCenterList& value)
{
    self.StartObject();
    self.Member("TaskCenterData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, TaskCenterList& value)
{
    self.StartObject();
    self.Member("TaskCenterData");
    std::size_t count = 0;

    self.StartArray(&count);
    TaskCenterData info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TaskCenterData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const TaskCenterData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const TaskCenterData& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, TaskCenterData& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator>>(QByteArray& self, TaskCenterMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const TaskCenterMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const TaskCenterMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, TaskCenterMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}

QString& operator<<(QString& self, const TaskCenterList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, TaskCenterList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const TaskCenterList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QByteArray& operator>>(QByteArray& self, TaskCenterList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
