#include "CommunicationSerialize.h"
#include "JsonHelper.h"
#include "Utility.h"
#include "CConverter.h"
#include <QDataStream>
#include <QDebug>

JsonWriter& operator&(JsonWriter& self, const CommunicationInfo& value)
{
    self.StartObject();
    self.Member("id") & value.id;
    self.Member("desc") & value.desc;
    self.Member("desIP") & value.desIP;
    self.Member("localIP") & value.localIP;
    self.Member("recvPort") & value.recvPort;
    self.Member("sendPort") & value.sendPort;
    self.Member("onlyRecvTargetData") & value.onlyRecvTargetData;
    self.Member("recvDataDesModule") & value.recvDataDesModule;
    self.Member("devProtocolCheck") & value.devProtocolCheck;
    QString tempStr;
    tempStr << value.type;
    self.Member("type") & tempStr;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, CommunicationInfo& value)
{
    self.StartObject();
    self.Member("id") & value.id;
    self.Member("desc") & value.desc;
    self.Member("desIP") & value.desIP;
    self.Member("localIP") & value.localIP;
    self.Member("recvPort") & value.recvPort;
    self.Member("sendPort") & value.sendPort;
    self.Member("onlyRecvTargetData") & value.onlyRecvTargetData;
    self.Member("recvDataDesModule") & value.recvDataDesModule;
    self.Member("devProtocolCheck") & value.devProtocolCheck;
    QString tempStr;
    self.Member("type") & tempStr;
    tempStr >> value.type;
    return self.EndObject();
}

JsonWriter& operator&(JsonWriter& self, const CommunicationInfoMap& value)
{
    self.StartObject();
    self.Member("ChannelNum") & value.channelNum;
    self.Member("CommunicationInfoData");

    self.StartArray();
    for (auto& item : value.infoMap)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, CommunicationInfoMap& value)
{
    self.StartObject();
    self.Member("ChannelNum") & value.channelNum;
    self.Member("CommunicationInfoData");

    std::size_t count = 0;

    self.StartArray(&count);
    CommunicationInfo info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.infoMap[info.id] = info;
    }
    self.EndArray();

    return self.EndObject();
}
JsonWriter& operator&(JsonWriter& self, const CommunicationInfoList& value)
{
    self.StartObject();
    self.Member("ChannelNum") & value.channelNum;
    self.Member("CommunicationInfoData");

    self.StartArray();
    for (auto& item : value.infoList)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, CommunicationInfoList& value)
{
    self.StartObject();
    self.Member("ChannelNum") & value.channelNum;
    self.Member("CommunicationInfoData");
    std::size_t count = 0;

    self.StartArray(&count);
    CommunicationInfo info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.infoList << info;
    }
    self.EndArray();

    return self.EndObject();
}

QString& operator<<(QString& self, CommunicationType value)
{
    switch (value)
    {
    case CommunicationType::Unknow:
    {
        return self;
    }
    case CommunicationType::UDPGB:
    {
        return self.append("UDPGB");
    }
    case CommunicationType::UDPP2P:
    {
        return self.append("UDPP2P");
    }
    case CommunicationType::TCPServer:
    {
        return self.append("TCPServer");
    }
    case CommunicationType::TCPClient:
    {
        return self.append("TCPClient");
    }
    }
    return self;
}
QString& operator>>(QString& self, CommunicationType& value)
{
    auto normalize = self.toUpper();
    if (normalize == "UDPGB")
    {
        value = CommunicationType::UDPGB;
    }
    else if (normalize == "UDPP2P")
    {
        value = CommunicationType::UDPP2P;
    }
    else if (normalize == "TCPServer")
    {
        value = CommunicationType::TCPServer;
    }
    else if (normalize == "TCPClient")
    {
        value = CommunicationType::TCPClient;
    }
    else
    {
        value = CommunicationType::Unknow;
    }
    return self;
}
QByteArray& operator<<(QByteArray& self, const CommunicationInfo& value)
{
    auto bak = value;
    JsonWriter writer;
    writer& bak;
    return self;
}

QByteArray& operator>>(QByteArray& self, CommunicationInfo& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QString& operator<<(QString& self, const CommunicationInfo& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetString());
    return self;
}
QString& operator>>(QString& self, CommunicationInfo& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const CommunicationInfoMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QByteArray& operator>>(QByteArray& self, CommunicationInfoMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QString& operator<<(QString& self, const CommunicationInfoMap& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, CommunicationInfoMap& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QByteArray& operator<<(QByteArray& self, const CommunicationInfoList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QByteArray& operator>>(QByteArray& self, CommunicationInfoList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}
QString& operator<<(QString& self, const CommunicationInfoList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, CommunicationInfoList& value)
{
    JsonReader reader(self);
    reader& value;

    return self;
}

QByteArray CommunicationHelper::toByteArray(qint32 channel, const QByteArray& data)
{
    QByteArray self;
    self.append(CConverter::toByteArray<qint32>(channel));
    self.append(data);
    return self;
}

void CommunicationHelper::fromByteArray(qint32& channel, QByteArray& data)
{
    channel = CConverter::fromByteArray<qint32>(data.mid(0, static_cast<qint32>(sizeof(channel))));
    data = data.mid(static_cast<qint32>(sizeof(channel)));
}

void CommunicationHelper::fromByteArray(const QByteArray& self, qint32& channel, QByteArray& data)
{
    channel = CConverter::fromByteArray<qint32>(self.mid(0, static_cast<qint32>(sizeof(channel))));
    data = self.mid(static_cast<qint32>(sizeof(channel)));
}

QDebug operator<<(QDebug self, const CommunicationInfo& value)
{
    QString type;
    type << value.type;
    auto str = QString("*****************************************\n通讯ID:%1\n发送端口:%2\n接收端口:%3\n描述:%4\n目标IP:%5\n本地IP:%6\n"
                       "通讯类型:%7\n是否只接收特定地址的数据:%8\n*****************************************\n")
                   .arg(value.id, 0, 16)
                   .arg(value.sendPort)
                   .arg(value.recvPort)
                   .arg(value.desc, value.desIP, value.localIP, type, value.onlyRecvTargetData ? "是" : "否");
    self.noquote() << str;
    return self;
}
