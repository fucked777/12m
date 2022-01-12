#ifndef PHASECALIBRATIONSERIALIZE_H
#define PHASECALIBRATIONSERIALIZE_H

#include "PhaseCalibrationDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationParameter& value);
JsonReader& operator&(JsonReader& self, PhaseCalibrationParameter& value);
QByteArray& operator>>(QByteArray& self, PhaseCalibrationParameter& value);
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationParameter& value);
QString& operator<<(QString& self, const PhaseCalibrationParameter& value);
QString& operator>>(QString& self, PhaseCalibrationParameter& value);

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationSaveParameter& value);
JsonReader& operator&(JsonReader& self, PhaseCalibrationSaveParameter& value);
QByteArray& operator>>(QByteArray& self, PhaseCalibrationSaveParameter& value);
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationSaveParameter& value);
QString& operator<<(QString& self, const PhaseCalibrationSaveParameter& value);
QString& operator>>(QString& self, PhaseCalibrationSaveParameter& value);

JsonWriter& operator&(JsonWriter& self, const StartPhaseCalibration& value);
JsonReader& operator&(JsonReader& self, StartPhaseCalibration& value);
QByteArray& operator>>(QByteArray& self, StartPhaseCalibration& value);
QByteArray& operator<<(QByteArray& self, const StartPhaseCalibration& value);
QString& operator<<(QString& self, const StartPhaseCalibration& value);
QString& operator>>(QString& self, StartPhaseCalibration& value);

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationItem& value);
JsonReader& operator&(JsonReader& self, PhaseCalibrationItem& value);
QByteArray& operator>>(QByteArray& self, PhaseCalibrationItem& value);
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationItem& value);
QString& operator<<(QString& self, const PhaseCalibrationItem& value);
QString& operator>>(QString& self, PhaseCalibrationItem& value);

JsonWriter& operator&(JsonWriter& self, const PhaseCalibrationItemList& value);
JsonReader& operator&(JsonReader& self, PhaseCalibrationItemList& value);
QByteArray& operator>>(QByteArray& self, PhaseCalibrationItemList& value);
QByteArray& operator<<(QByteArray& self, const PhaseCalibrationItemList& value);
QString& operator<<(QString& self, const PhaseCalibrationItemList& value);
QString& operator>>(QString& self, PhaseCalibrationItemList& value);

JsonWriter& operator&(JsonWriter& self, const ClearPhaseCalibrationHistry& value);
JsonReader& operator&(JsonReader& self, ClearPhaseCalibrationHistry& value);
QByteArray& operator>>(QByteArray& self, ClearPhaseCalibrationHistry& value);
QByteArray& operator<<(QByteArray& self, const ClearPhaseCalibrationHistry& value);
QString& operator<<(QString& self, const ClearPhaseCalibrationHistry& value);
QString& operator>>(QString& self, ClearPhaseCalibrationHistry& value);

JsonWriter& operator&(JsonWriter& self, const QueryPhaseCalibrationHistry& value);
JsonReader& operator&(JsonReader& self, QueryPhaseCalibrationHistry& value);
QByteArray& operator>>(QByteArray& self, QueryPhaseCalibrationHistry& value);
QByteArray& operator<<(QByteArray& self, const QueryPhaseCalibrationHistry& value);
QString& operator<<(QString& self, const QueryPhaseCalibrationHistry& value);
QString& operator>>(QString& self, QueryPhaseCalibrationHistry& value);

class PhaseCalibrationHelper
{
public:
    static QString toString(PhaseCalibrationStatus status);
    static QString toString(ACUPhaseCalibrationFreqBand status);
    static QString toString(SelfTrackCheckResult status);
    /* 分解成单模式 */
    static QList<ACUPhaseCalibrationFreqBand> split(ACUPhaseCalibrationFreqBand status);
};
Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline uint qHash(const ACUPhaseCalibrationFreqBand& key, uint seed = 0) Q_DECL_NOTHROW
{
    return qHash(static_cast<int>(key), seed);
}

#endif  // PHASECALIBRATIONSERIALIZE_H
