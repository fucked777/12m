#ifndef EPHEMERISDATASERIALIZE_H
#define EPHEMERISDATASERIALIZE_H
#include "EphemerisDataDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const EphemerisDataData& value);
JsonReader& operator&(JsonReader& self, EphemerisDataData& value);
JsonWriter& operator&(JsonWriter& self, const EphemerisDataMap& value);
JsonReader& operator&(JsonReader& self, EphemerisDataMap& value);
JsonWriter& operator&(JsonWriter& self, const EphemerisDataList& value);
JsonReader& operator&(JsonReader& self, EphemerisDataList& value);

QByteArray& operator>>(QByteArray& self, EphemerisDataData& value);
QByteArray& operator<<(QByteArray& self, const EphemerisDataData& value);
QString& operator<<(QString& self, const EphemerisDataData& value);
QString& operator>>(QString& self, EphemerisDataData& value);

QByteArray& operator>>(QByteArray& self, EphemerisDataMap& value);
QByteArray& operator<<(QByteArray& self, const EphemerisDataMap& value);
QString& operator<<(QString& self, const EphemerisDataMap& value);
QString& operator>>(QString& self, EphemerisDataMap& value);

QByteArray& operator>>(QByteArray& self, EphemerisDataList& value);
QByteArray& operator<<(QByteArray& self, const EphemerisDataList& value);
QString& operator<<(QString& self, const EphemerisDataList& value);
QString& operator>>(QString& self, EphemerisDataList& value);

#endif  // EPHEMERISDATASERIALIZE_H
