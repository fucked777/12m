#ifndef ResourceRestructuringMESSAGESERIALIZE2_H
#define ResourceRestructuringMESSAGESERIALIZE2_H

#include "ResourceRestructuringMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const QMap<int, DeviceID>& data);
JsonReader& operator&(JsonReader& self, QMap<int, DeviceID>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, DeviceID>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, DeviceID>& data);
QString& operator>>(QString& self, QMap<int, DeviceID>& data);
QString& operator<<(QString& self, const QMap<int, DeviceID>& data);

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringData& data);
JsonReader& operator&(JsonReader& self, ResourceRestructuringData& data);
QByteArray& operator>>(QByteArray& self, ResourceRestructuringData& data);
QByteArray& operator<<(QByteArray& self, const ResourceRestructuringData& data);
QString& operator>>(QString& self, ResourceRestructuringData& data);
QString& operator<<(QString& self, const ResourceRestructuringData& data);

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringDataMap& data);
JsonReader& operator&(JsonReader& self, ResourceRestructuringDataMap& data);
QByteArray& operator>>(QByteArray& self, ResourceRestructuringDataMap& data);
QByteArray& operator<<(QByteArray& self, const ResourceRestructuringDataMap& data);
QString& operator>>(QString& self, ResourceRestructuringDataMap& data);
QString& operator<<(QString& self, const ResourceRestructuringDataMap& data);

JsonWriter& operator&(JsonWriter& self, const ResourceRestructuringDataList& data);
JsonReader& operator&(JsonReader& self, ResourceRestructuringDataList& data);
QByteArray& operator>>(QByteArray& self, ResourceRestructuringDataList& data);
QByteArray& operator<<(QByteArray& self, const ResourceRestructuringDataList& data);
QString& operator>>(QString& self, ResourceRestructuringDataList& data);
QString& operator<<(QString& self, const ResourceRestructuringDataList& data);

#endif  // ResourceRestructuringMESSAGESERIALIZE2_H
