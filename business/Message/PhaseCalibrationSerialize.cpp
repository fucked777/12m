#include "PhaseCalibrationSerialize.h"

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationParameter& value)
{
    //    self.Member("equipComb") & value.equipComb;
    //    self.Member("sEmissPolar") & value.sEmissPolar;
    //    self.Member("kaSCEmissPolar") & value.kaSCEmissPolar;
    //    self.Member("kaYCEmissPolar") & value.kaYCEmissPolar;

    self.StartObject();

    self.Member("projectCode") & value.projectCode;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("taskCode") & value.taskCode;
    self.Member("freqBand") & value.freqBand;
    self.Member("corrMeth") & value.corrMeth;
    self.Member("followCheck") & value.followCheck;
    self.Member("crossLowLimit") & value.crossLowLimit;
    self.Member("standSensit") & value.standSensit;
    self.Member("directToler") & value.directToler;

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, PhaseCalibrationParameter& value)
{
    self.StartObject();

    self.Member("projectCode") & value.projectCode;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("taskCode") & value.taskCode;
    self.Member("freqBand") & value.freqBand;
    self.Member("corrMeth") & value.corrMeth;
    self.Member("followCheck") & value.followCheck;
    self.Member("crossLowLimit") & value.crossLowLimit;
    self.Member("standSensit") & value.standSensit;
    self.Member("directToler") & value.directToler;

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, PhaseCalibrationParameter& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationParameter& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const PhaseCalibrationParameter& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, PhaseCalibrationParameter& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationSaveParameter& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationSaveParameter");

    self.StartObject();
    self.Member("freqBand") & value.freqBand;
    self.Member("systemWorkMode") & value.systemWorkMode;
    self.Member("dotDrequency") & value.dotDrequency;
    self.Member("downFreq") & value.downFreq;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, PhaseCalibrationSaveParameter& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationSaveParameter");

    self.StartObject();
    self.Member("freqBand") & value.freqBand;
    self.Member("systemWorkMode") & value.systemWorkMode;
    self.Member("dotDrequency") & value.dotDrequency;
    self.Member("downFreq") & value.downFreq;
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, PhaseCalibrationSaveParameter& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationSaveParameter& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const PhaseCalibrationSaveParameter& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, PhaseCalibrationSaveParameter& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const StartPhaseCalibration& value)
{
    self.StartObject();
    self.Member("parameter") & value.parameter;

    self.Member("saveParameter");
    self.StartArray();
    for (auto& item : value.saveParameter)
    {
        self& item;
    }
    self.EndArray();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, StartPhaseCalibration& value)
{
    self.StartObject();
    self.Member("parameter") & value.parameter;

    self.Member("saveParameter");
    std::size_t count = 0;
    PhaseCalibrationSaveParameter parameter;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        self& parameter;
        value.saveParameter.insert(parameter.freqBand, parameter);
    }
    self.EndArray();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, StartPhaseCalibration& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const StartPhaseCalibration& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const StartPhaseCalibration& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, StartPhaseCalibration& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationItem& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationItem");

    self.StartObject();
    self.Member("id") & value.id;
    self.Member("projectCode") & value.projectCode;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("taskCode") & value.taskCode;
    self.Member("createTime") & value.createTime;
    self.Member("workMode") & value.workMode;
    self.Member("equipComb") & value.equipComb;
    self.Member("dotDrequency") & value.dotDrequency;
    self.Member("downFreq") & value.downFreq;
    self.Member("temperature") & value.temperature;
    self.Member("humidity") & value.humidity;
    self.Member("azpr") & value.azpr;
    self.Member("elpr") & value.elpr;
    self.Member("azge") & value.azge;
    self.Member("elge") & value.elge;
    self.Member("rawParameter") & value.rawParameter;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, PhaseCalibrationItem& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationItem");

    self.StartObject();
    self.Member("id") & value.id;
    self.Member("projectCode") & value.projectCode;
    self.Member("taskIdent") & value.taskIdent;
    self.Member("taskCode") & value.taskCode;
    self.Member("createTime") & value.createTime;
    self.Member("workMode") & value.workMode;
    self.Member("equipComb") & value.equipComb;
    self.Member("dotDrequency") & value.dotDrequency;
    self.Member("downFreq") & value.downFreq;
    self.Member("temperature") & value.temperature;
    self.Member("humidity") & value.humidity;
    self.Member("azpr") & value.azpr;
    self.Member("elpr") & value.elpr;
    self.Member("azge") & value.azge;
    self.Member("elge") & value.elge;
    self.Member("rawParameter") & value.rawParameter;
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, PhaseCalibrationItem& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationItem& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const PhaseCalibrationItem& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, PhaseCalibrationItem& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationItemList& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationItemData");

    self.StartArray();
    for (auto& item : value)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, PhaseCalibrationItemList& value)
{
    self.StartObject();
    self.Member("PhaseCalibrationItemData");
    std::size_t count = 0;

    self.StartArray(&count);
    PhaseCalibrationItem info;
    for (std::size_t i = 0; i < count; i++)
    {
        self& info;
        value.append(info);
    }
    self.EndArray();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, PhaseCalibrationItemList& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationItemList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const PhaseCalibrationItemList& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, PhaseCalibrationItemList& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ClearPhaseCalibrationHistry& value)
{
    self.StartObject();
    self.Member("ClearPhaseCalibrationHistry");

    self.StartObject();
    self.Member("timeInterval") & value.timeInterval;
    self.EndObject();

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ClearPhaseCalibrationHistry& value)
{
    self.StartObject();
    self.Member("ClearPhaseCalibrationHistry");

    self.StartObject();
    self.Member("timeInterval") & value.timeInterval;
    self.EndObject();

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ClearPhaseCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ClearPhaseCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const ClearPhaseCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, ClearPhaseCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

JsonWriter& operator&(JsonWriter& self, const QueryPhaseCalibrationHistry& value)
{
    self.StartObject();
    self.Member("taskCode") & value.taskCode;
    self.Member("beginTime") & value.beginTime;
    self.Member("endTime") & value.endTime;
    self.Member("workMode") & value.workMode;

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, QueryPhaseCalibrationHistry& value)
{
    self.StartObject();
    self.Member("taskCode") & value.taskCode;
    self.Member("beginTime") & value.beginTime;
    self.Member("endTime") & value.endTime;
    self.Member("workMode") & value.workMode;

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, QueryPhaseCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}
QByteArray& operator<<(QByteArray& self, const QueryPhaseCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator<<(QString& self, const QueryPhaseCalibrationHistry& value)
{
    JsonWriter writer;
    writer& value;
    self.append(writer.GetQString());
    return self;
}
QString& operator>>(QString& self, QueryPhaseCalibrationHistry& value)
{
    JsonReader reader(self);
    reader& value;
    return self;
}

QString PhaseCalibrationHelper::toString(PhaseCalibrationStatus status)
{
    switch (status)
    {
    case PhaseCalibrationStatus::Start:
    {
        return "开始校相";
    }
    case PhaseCalibrationStatus::Finish:
    {
        return "校相正常完成";
    }
    case PhaseCalibrationStatus::End:
    {
        return "结束校相";
    }
    case PhaseCalibrationStatus::NotFoundTrack:
    {
        return "未找到轨道数据";
    }
    case PhaseCalibrationStatus::BasebandQueryTimeout:
    {
        return "基带查询超时";
    }
    case PhaseCalibrationStatus::BasebandPhaseCalibrationTimeout:
    {
        return "基带校相超时";
    }
    case PhaseCalibrationStatus::BasebandBindingTimeout:
    {
        return "基带装订超时";
    }
    case PhaseCalibrationStatus::BasebandRejectionPhaseCalibration:
    {
        return "基带拒收校相";
    }
    case PhaseCalibrationStatus::BasebandRejectionBinding:
    {
        return "基带拒收装订";
    }
    case PhaseCalibrationStatus::ReceiverOutOfLock:
    {
        return "接收机失锁";
    }
    case PhaseCalibrationStatus::StartSelfTrackingCheck:
    {
        return "开始自跟踪检查";
    }
    case PhaseCalibrationStatus::FinishSelfTrackingCheck:
    {
        return "完成自跟踪检查";
    }
    case PhaseCalibrationStatus::StartOSAndCCCheck:
    {
        return "开始定向灵敏度和交叉耦合检查";
    }
    case PhaseCalibrationStatus::FinishOSAndCCCheck:
    {
        return "完成定向灵敏度和交叉耦合检查";
    }
    case PhaseCalibrationStatus::AGCAbnormal:
    {
        return "AGC异常";
    }
    case PhaseCalibrationStatus::ErrorVoltageOverrun:
    {
        return "误差电压超限";
    }
    case PhaseCalibrationStatus::BasebandBindingFailed:
    {
        return "基带装订失败";
    }
    case PhaseCalibrationStatus::Error: break;
    }
    return "ACU未知错误";
}
QString PhaseCalibrationHelper::toString(ACUPhaseCalibrationFreqBand fb)
{
    switch (fb)
    {
    case ACUPhaseCalibrationFreqBand::S:
    {
        return "S";
    }
    case ACUPhaseCalibrationFreqBand::KaYC:
    {
        return "Ka遥测";
    }
    case ACUPhaseCalibrationFreqBand::KaSC:
    {
        return "Ka数传";
    }
    case ACUPhaseCalibrationFreqBand::SKaYC:
    {
        return "S+Ka遥测";
    }
    case ACUPhaseCalibrationFreqBand::SKaSC:
    {
        return "S+Ka数传";
    }
    case ACUPhaseCalibrationFreqBand::SKaYCKaSC:
    {
        return "S+Ka遥测+Ka数传";
    }
    case ACUPhaseCalibrationFreqBand::Unknown: break;
    }
    return "未知";
}
QString PhaseCalibrationHelper::toString(SelfTrackCheckResult status)
{
    switch (status)
    {
    case SelfTrackCheckResult::NormalPolarity:
    {
        return "极性正常";
    }
    case SelfTrackCheckResult::AZPolarityReversal:
    {
        return "方位极性反";
    }
    case SelfTrackCheckResult::ELPolarityReversal:
    {
        return "俯仰极性反";
    }
    case SelfTrackCheckResult::AZELPolarityReversal:
    {
        return "方位俯仰极性均反";
    }
    case SelfTrackCheckResult::OutOfLock:
    {
        return "检查时失锁";
    }
    case SelfTrackCheckResult::AGCAbnormal:
    {
        return "AGC异常";
    }
    case SelfTrackCheckResult::AZRandomDifferenceAbnormal:
    {
        return "方位随机差异常";
    }
    case SelfTrackCheckResult::ElRandomDifferenceAbnormal:
    {
        return "俯仰随机差异常";
    }
    case SelfTrackCheckResult::NoCheck:
    {
        return "未做检查";
    }
    }
    return "未知";
}
QList<ACUPhaseCalibrationFreqBand> PhaseCalibrationHelper::split(ACUPhaseCalibrationFreqBand fb)
{
    QList<ACUPhaseCalibrationFreqBand> result;
    switch (fb)
    {
    case ACUPhaseCalibrationFreqBand::S:
    case ACUPhaseCalibrationFreqBand::KaYC:
    case ACUPhaseCalibrationFreqBand::KaSC:
    {
        result << fb;
        return result;
    }
    case ACUPhaseCalibrationFreqBand::SKaYC:
    {
        result << ACUPhaseCalibrationFreqBand::S;
        result << ACUPhaseCalibrationFreqBand::KaYC;
        return result;
    }
    case ACUPhaseCalibrationFreqBand::SKaSC:
    {
        result << ACUPhaseCalibrationFreqBand::S;
        result << ACUPhaseCalibrationFreqBand::KaSC;
        return result;
    }
    case ACUPhaseCalibrationFreqBand::SKaYCKaSC:
    {
        result << ACUPhaseCalibrationFreqBand::S;
        result << ACUPhaseCalibrationFreqBand::KaYC;
        result << ACUPhaseCalibrationFreqBand::KaSC;
        return result;
    }
    case ACUPhaseCalibrationFreqBand::Unknown: break;
    }
    return result;
}
