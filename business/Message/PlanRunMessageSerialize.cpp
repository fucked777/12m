#include "PlanRunMessageSerialize.h"
#include "ConfigMacroMessageSerialize.h"
#include "ParamMacroMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const TargetInfo& data)
{
    self.StartObject();

    self.Member("TargetNo") & data.targetNo;
    self.Member("WorkMode") & static_cast<int>(data.workMode);
    self.Member("TaskCode") & data.taskCode;
    self.Member("PointFreqNo") & data.pointFreqNo;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, TargetInfo& data)
{
    self.StartObject();

    self.Member("TargetNo") & data.targetNo;

    int workMode = 0;
    self.Member("WorkMode") & workMode;
    data.workMode = (SystemWorkMode)workMode;

    self.Member("TaskCode") & data.taskCode;
    self.Member("PointFreqNo") & data.pointFreqNo;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, TargetInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const TargetInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, TargetInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const TargetInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<int, TargetInfo>& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number(key)) & value;
    }
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<int, TargetInfo>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto& key : keys)
    {
        TargetInfo targetInfo;
        self.Member(key) & targetInfo;

        data[key.toInt()] = targetInfo;
    }
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<int, TargetInfo>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<int, TargetInfo>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<int, TargetInfo>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QMap<int, TargetInfo>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const LinkLine& data)
{
    self.StartObject();
    self.Member("LinkType") & data.linkType;
    self.Member("WorkMode") & static_cast<int>(data.workMode);
    self.Member("MasterTargetNo") & data.masterTargetNo;
    self.Member("MasterType") & data.masterType;

    self.Member("TargetMap") & data.targetMap;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, LinkLine& data)
{
    self.StartObject();
    self.Member("LinkType") & data.linkType;

    int workMode = 0;
    self.Member("WorkMode") & workMode;
    data.workMode = (SystemWorkMode)workMode;

    self.Member("MasterTargetNo") & data.masterTargetNo;
    self.Member("MasterType") & data.masterType;

    self.Member("TargetMap") & data.targetMap;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, LinkLine& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const LinkLine& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, LinkLine& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const LinkLine& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, LinkLine>& data)
{
    self.StartObject();

    for (auto key : data.keys())
    {
        auto value = data.value(key);
        self.Member(QString::number(int(key))) & value;
    }
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, LinkLine>& data)
{
    self.StartObject();
    auto keys = self.Members();
    for (auto& key : keys)
    {
        LinkLine linkLine;
        self.Member(key) & linkLine;

        auto workMode = SystemWorkMode(key.toInt());
        data[workMode] = linkLine;
    }
    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, LinkLine>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, LinkLine>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, QMap<SystemWorkMode, LinkLine>& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const QMap<SystemWorkMode, LinkLine>& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ManualMessage& data)
{
    self.StartObject();

    self.Member("ManualType") & data.manualType;
    self.Member("LinkMasterSlave") & data.linkMasterSlave;

    self.Member("IsManualControl") & data.isManualContrl;
    self.Member("ConfigMacroData") & data.configMacroData;

    self.Member("IsExternalParamMacro") & data.isExternalParamMacro;
    self.Member("ParamMacroData") & data.paramMacroData;

    self.Member("AcuParam") & data.acuParam;
    self.Member("LtDTETask") & data.ltDTETask;
    self.Member("DteTraceStartDateTime") & data.dteTraceStartDateTime;
    self.Member("DteTaskEndDateTime") & data.dteTaskEndDateTime;

    self.Member("ResourceReleaseDeleteDTETask") & data.resourceReleaseDeleteDTETask;
    self.Member("ResourceReleaseDeleteCZTask") & data.resourceReleaseDeleteCZTask;
    self.Member("ResourceReleaseDeleteACUTask") & data.resourceReleaseDeleteACUTask;

    self.Member("IsTest") & data.isTest;

    self.Member("XlMode") & data.xlMode;
    self.Member("XXMode") & data.xxMode;

    self.Member("LinkLineMap") & data.linkLineMap;

    self.Member("DevNumber") & data.devNumber;

    self.Member("MasterTaskCode") & data.masterTaskCode;
    self.Member("MasterTaskBz") & data.masterTaskBz;

    self.Member("TxUACAddr") & data.txUACAddr;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, ManualMessage& data)
{
    self.StartObject();

    self.Member("ManualType") & data.manualType;
    self.Member("LinkMasterSlave") & data.linkMasterSlave;

    self.Member("IsManualControl") & data.isManualContrl;
    self.Member("ConfigMacroData") & data.configMacroData;

    self.Member("IsExternalParamMacro") & data.isExternalParamMacro;
    self.Member("ParamMacroData") & data.paramMacroData;

    self.Member("AcuParam") & data.acuParam;
    self.Member("LtDTETask") & data.ltDTETask;
    self.Member("DteTraceStartDateTime") & data.dteTraceStartDateTime;
    self.Member("DteTaskEndDateTime") & data.dteTaskEndDateTime;

    self.Member("ResourceReleaseDeleteDTETask") & data.resourceReleaseDeleteDTETask;
    self.Member("ResourceReleaseDeleteCZTask") & data.resourceReleaseDeleteCZTask;
    self.Member("ResourceReleaseDeleteACUTask") & data.resourceReleaseDeleteACUTask;

    self.Member("IsTest") & data.isTest;

    self.Member("XlMode") & data.xlMode;
    self.Member("XXMode") & data.xxMode;

    self.Member("LinkLineMap") & data.linkLineMap;

    self.Member("DevNumber") & data.devNumber;

    self.Member("MasterTaskCode") & data.masterTaskCode;
    self.Member("MasterTaskBz") & data.masterTaskBz;

    self.Member("TxUACAddr") & data.txUACAddr;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, ManualMessage& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const ManualMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, ManualMessage& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const ManualMessage& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
