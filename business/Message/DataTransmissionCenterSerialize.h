#ifndef DATATRANSMISSIONCENTERSERIALIZE_H
#define DATATRANSMISSIONCENTERSERIALIZE_H
#include "DataTransmissionCenterDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterData& value);
JsonReader& operator&(JsonReader& self, DataTransmissionCenterData& value);
JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterMap& value);
JsonReader& operator&(JsonReader& self, DataTransmissionCenterMap& value);
JsonWriter& operator&(JsonWriter& self, const DataTransmissionCenterList& value);
JsonReader& operator&(JsonReader& self, DataTransmissionCenterList& value);

QByteArray& operator>>(QByteArray& self, DataTransmissionCenterData& value);
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterData& value);
QString& operator<<(QString& self, const DataTransmissionCenterData& value);
QString& operator>>(QString& self, DataTransmissionCenterData& value);

QByteArray& operator>>(QByteArray& self, DataTransmissionCenterMap& value);
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterMap& value);
QString& operator<<(QString& self, const DataTransmissionCenterMap& value);
QString& operator>>(QString& self, DataTransmissionCenterMap& value);

QByteArray& operator>>(QByteArray& self, DataTransmissionCenterList& value);
QByteArray& operator<<(QByteArray& self, const DataTransmissionCenterList& value);
QString& operator<<(QString& self, const DataTransmissionCenterList& value);
QString& operator>>(QString& self, DataTransmissionCenterList& value);

#endif  // DATATRANSMISSIONCENTERSERIALIZE_H
