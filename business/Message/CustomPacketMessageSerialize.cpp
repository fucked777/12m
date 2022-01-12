#include "CustomPacketMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const UnitReportMessage& data)
{
    self.StartObject();
    self.Member("id") & data.id;
    self.Member("paramMap");

    self.StartObject();
    for (auto key : data.paramMap.keys())
    {
        auto value = data.paramMap.value(key);
        self.Member(key) & value;
    }
    self.EndObject();

    self.Member("multiTargetParamMap");
    self.StartObject();
    for (auto targetNo : data.multiTargetParamMap.keys())
    {
        self.Member(QString::number(targetNo));

        self.StartObject();
        auto targetParamMap = data.multiTargetParamMap.value(targetNo);
        for (auto paramId : targetParamMap.keys())
        {
            auto value = targetParamMap.value(paramId);
            self.Member(paramId) & value;
        }
        self.EndObject();
    }
    self.EndObject();

    self.Member("multiTargetParamMap2");
    self.StartObject();
    for (auto targetNo : data.multiTargetParamMap2.keys())
    {
        self.Member(QString::number(targetNo));

        self.StartArray();
        auto recycleParamList = data.multiTargetParamMap2.value(targetNo);

        for (auto paramMap : recycleParamList)
        {
            self.StartObject();
            for (auto paramId : paramMap.keys())
            {
                auto value = paramMap.value(paramId);
                self.Member(paramId) & value;
            }
            self.EndObject();
        }

        self.EndArray();
    }
    self.EndObject();

    self.Member("multiTargetParamMap3");
    self.StartObject();
    for (auto targetNo1 : data.multiTargetParamMap3.keys())
    {
        self.Member(QString::number(targetNo1));
        self.StartObject();

        auto targetMap = data.multiTargetParamMap3.value(targetNo1);
        for (auto targetNo2 : targetMap.keys())
        {
            self.Member(QString::number(targetNo2));

            self.StartArray();
            auto recycleParamList = targetMap.value(targetNo2);

            for (auto paramMap : recycleParamList)
            {
                self.StartObject();
                for (auto paramId : paramMap.keys())
                {
                    auto value = paramMap.value(paramId);
                    self.Member(paramId) & value;
                }
                self.EndObject();
            }

            self.EndArray();
        }
        self.EndObject();
    }
    self.EndObject();

    self.Member("dteTaskBZAndCenterBZMap");
    self.StartObject();
    for (auto taskBZ : data.dteTaskBZAndCenterBZMap.keys())
    {
        self.Member(taskBZ.toString());

        self.StartObject();
        auto targetParamList = data.dteTaskBZAndCenterBZMap.value(taskBZ);
        int count = 0;
        for (auto centerBZ : targetParamList)
        {
            self.Member(QString::number(count++)) & centerBZ;
        }
        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, UnitReportMessage& data)
{
    self.StartObject();

    self.Member("id") & data.id;

    self.Member("paramMap");
    self.StartObject();
    auto paramIds = self.Members();
    for (auto paramId : paramIds)
    {
        self.Member(paramId) & data.paramMap[paramId];
    }
    self.EndObject();

    self.Member("multiTargetParamMap");
    self.StartObject();
    auto targetNos = self.Members();
    for (auto targetNo : targetNos)
    {
        self.Member(targetNo);
        self.StartObject();
        auto paramIds2 = self.Members();
        for (auto paramId : paramIds2)
        {
            self.Member(paramId) & data.multiTargetParamMap[targetNo.toInt()][paramId];
        }
        self.EndObject();
    }
    self.EndObject();

    self.Member("multiTargetParamMap2");
    self.StartObject();
    auto targetNos1 = self.Members();
    for (auto targetNo : targetNos1)
    {
        QList<QMap<QString, QVariant>> paramMapList;

        self.Member(targetNo);
        std::size_t count = 0;
        self.StartArray(&count);
        for (std::size_t i = 0; i < count; i++)
        {
            QMap<QString, QVariant> paramMap;
            self.StartObject();
            auto paramIds3 = self.Members();
            for (auto paramId : paramIds3)
            {
                self.Member(paramId) & paramMap[paramId];
            }
            self.EndObject();
            paramMapList << paramMap;
        }
        self.EndArray();

        data.multiTargetParamMap2[targetNo.toInt()] = paramMapList;
    }
    self.EndObject();

    self.Member("multiTargetParamMap3");
    self.StartObject();
    auto targetNos11 = self.Members();
    for (auto targetNo1 : targetNos11)
    {
        QMap<int, QList<QMap<QString, QVariant>>> targetParamListMap;

        self.Member(targetNo1);
        self.StartObject();
        auto targetNos2 = self.Members();
        for (auto targetNo2 : targetNos2)
        {
            QList<QMap<QString, QVariant>> paramMapList;

            self.Member(targetNo2);
            std::size_t count = 0;
            self.StartArray(&count);
            for (std::size_t i = 0; i < count; i++)
            {
                QMap<QString, QVariant> paramMap;
                self.StartObject();
                auto paramIds4 = self.Members();
                for (auto paramId : paramIds4)
                {
                    self.Member(paramId) & paramMap[paramId];
                }
                self.EndObject();
                paramMapList << paramMap;
            }
            self.EndArray();

            targetParamListMap[targetNo2.toInt()] = paramMapList;
        }
        self.EndObject();

        data.multiTargetParamMap3[targetNo1.toInt()] = targetParamListMap;
    }
    self.EndObject();

    self.Member("dteTaskBZAndCenterBZMap");
    self.StartObject();
    auto taskBZS = self.Members();
    for (auto taskBZ : taskBZS)
    {
        self.Member(taskBZ);
        self.StartObject();
        auto paramList = self.Members();
        for (auto paramId : paramList)
        {
            QVariant temp;
            self.Member(paramId) & temp;
            data.dteTaskBZAndCenterBZMap[taskBZ].append(temp);
        }
        self.EndObject();
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator<<(QByteArray& self, const UnitReportMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QByteArray& operator>>(QByteArray& self, UnitReportMessage& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator>>(QString& self, UnitReportMessage& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const UnitReportMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ExtensionStatusReportMessage& data)
{
    self.StartObject();
    self.Member("modeId") & data.modeId;
    self.Member("unitReportMsgMap");

    self.StartObject();
    for (auto unitId : data.unitReportMsgMap.keys())
    {
        auto unitReportMessage = data.unitReportMsgMap.value(unitId);
        self.Member(QString::number(unitId)) & unitReportMessage;
    }
    self.EndObject();

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ExtensionStatusReportMessage& data)
{
    self.StartObject();

    self.Member("modeId") & data.modeId;
    self.Member("unitReportMsgMap");

    self.StartObject();
    auto keys = self.Members();
    for (auto key : keys)
    {
        UnitReportMessage unitReportMsg;
        self.Member(key) & unitReportMsg;

        data.unitReportMsgMap[key.toInt()] = unitReportMsg;
    }
    self.EndObject();

    return self.EndObject();
}

QByteArray& operator<<(QByteArray& self, const ExtensionStatusReportMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QByteArray& operator>>(QByteArray& self, ExtensionStatusReportMessage& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator>>(QString& self, ExtensionStatusReportMessage& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const ExtensionStatusReportMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
