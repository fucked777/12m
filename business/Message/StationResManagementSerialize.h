#ifndef STATIONRESMANAGEMENTSERIALIZE_H
#define STATIONRESMANAGEMENTSERIALIZE_H
#include "StationResManagementDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const StationResManagementData& value);
JsonReader& operator&(JsonReader& self, StationResManagementData& value);
JsonWriter& operator&(JsonWriter& self, const StationResManagementMap& value);
JsonReader& operator&(JsonReader& self, StationResManagementMap& value);
JsonWriter& operator&(JsonWriter& self, const StationResManagementList& value);
JsonReader& operator&(JsonReader& self, StationResManagementList& value);

QByteArray& operator>>(QByteArray& self, StationResManagementData& value);
QByteArray& operator<<(QByteArray& self, const StationResManagementData& value);
QString& operator<<(QString& self, const StationResManagementData& value);
QString& operator>>(QString& self, StationResManagementData& value);

QByteArray& operator>>(QByteArray& self, StationResManagementMap& value);
QByteArray& operator<<(QByteArray& self, const StationResManagementMap& value);
QString& operator<<(QString& self, const StationResManagementMap& value);
QString& operator>>(QString& self, StationResManagementMap& value);

QByteArray& operator>>(QByteArray& self, StationResManagementList& value);
QByteArray& operator<<(QByteArray& self, const StationResManagementList& value);
QString& operator<<(QString& self, const StationResManagementList& value);
QString& operator>>(QString& self, StationResManagementList& value);

#endif  // STATIONRESMANAGEMENTSERIALIZE_H
