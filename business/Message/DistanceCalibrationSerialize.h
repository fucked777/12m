#ifndef DISTANCECALIBRATIONSERIALIZE_H
#define DISTANCECALIBRATIONSERIALIZE_H

#include "DistanceCalibrationDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const DistanceCalibrationItem& value);
JsonReader& operator&(JsonReader& self, DistanceCalibrationItem& value);
JsonWriter& operator&(JsonWriter& self, const DistanceCalibrationItemList& value);
JsonReader& operator&(JsonReader& self, DistanceCalibrationItemList& value);
JsonWriter& operator&(JsonWriter& self, const ClearDistanceCalibrationHistry& value);
JsonReader& operator&(JsonReader& self, ClearDistanceCalibrationHistry& value);
JsonWriter& operator&(JsonWriter& self, const QueryDistanceCalibrationHistry& value);
JsonReader& operator&(JsonReader& self, QueryDistanceCalibrationHistry& value);

QByteArray& operator>>(QByteArray& self, DistanceCalibrationItem& value);
QByteArray& operator<<(QByteArray& self, const DistanceCalibrationItem& value);
QString& operator<<(QString& self, const DistanceCalibrationItem& value);
QString& operator>>(QString& self, DistanceCalibrationItem& value);

QByteArray& operator>>(QByteArray& self, DistanceCalibrationItemList& value);
QByteArray& operator<<(QByteArray& self, const DistanceCalibrationItemList& value);
QString& operator<<(QString& self, const DistanceCalibrationItemList& value);
QString& operator>>(QString& self, DistanceCalibrationItemList& value);

QByteArray& operator>>(QByteArray& self, ClearDistanceCalibrationHistry& value);
QByteArray& operator<<(QByteArray& self, const ClearDistanceCalibrationHistry& value);
QString& operator<<(QString& self, const ClearDistanceCalibrationHistry& value);
QString& operator>>(QString& self, ClearDistanceCalibrationHistry& value);

QByteArray& operator>>(QByteArray& self, QueryDistanceCalibrationHistry& value);
QByteArray& operator<<(QByteArray& self, const QueryDistanceCalibrationHistry& value);
QString& operator<<(QString& self, const QueryDistanceCalibrationHistry& value);
QString& operator>>(QString& self, QueryDistanceCalibrationHistry& value);

#endif  // DISTANCECALIBRATIONSERIALIZE_H
