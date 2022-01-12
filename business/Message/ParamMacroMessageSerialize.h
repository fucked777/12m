#ifndef PARAMMACROMESSAGESERIALIZE_H
#define PARAMMACROMESSAGESERIALIZE_H

#include "ParamMacroMessage.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const QMap<int, QMap<QString, QVariant>>& data);
JsonReader& operator&(JsonReader& self, QMap<int, QMap<QString, QVariant>>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, QMap<QString, QVariant>>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, QMap<QString, QVariant>>& data);
QString& operator>>(QString& self, QMap<int, QMap<QString, QVariant>>& data);
QString& operator<<(QString& self, const QMap<int, QMap<QString, QVariant>>& data);

JsonWriter& operator&(JsonWriter& self, const DeviceData& data);
JsonReader& operator&(JsonReader& self, DeviceData& data);
QByteArray& operator>>(QByteArray& self, DeviceData& data);
QByteArray& operator<<(QByteArray& self, const DeviceData& data);
QString& operator>>(QString& self, DeviceData& data);
QString& operator<<(QString& self, const DeviceData& data);

JsonWriter& operator&(JsonWriter& self, const QMap<DeviceID, DeviceData>& data);
JsonReader& operator&(JsonReader& self, QMap<DeviceID, DeviceData>& data);
QByteArray& operator>>(QByteArray& self, QMap<DeviceID, DeviceData>& data);
QByteArray& operator<<(QByteArray& self, const QMap<DeviceID, DeviceData>& data);
QString& operator>>(QString& self, QMap<DeviceID, DeviceData>& data);
QString& operator<<(QString& self, const QMap<DeviceID, DeviceData>& data);

JsonWriter& operator&(JsonWriter& self, const ParamMacroModeData& data);
JsonReader& operator&(JsonReader& self, ParamMacroModeData& data);
QByteArray& operator>>(QByteArray& self, ParamMacroModeData& data);
QByteArray& operator<<(QByteArray& self, const ParamMacroModeData& data);
QString& operator>>(QString& self, ParamMacroModeData& data);
QString& operator<<(QString& self, const ParamMacroModeData& data);

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, ParamMacroModeData>& data);
JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, ParamMacroModeData>& data);
QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, ParamMacroModeData>& data);
QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, ParamMacroModeData>& data);
QString& operator>>(QString& self, QMap<SystemWorkMode, ParamMacroModeData>& data);
QString& operator<<(QString& self, const QMap<SystemWorkMode, ParamMacroModeData>& data);

JsonWriter& operator&(JsonWriter& self, const ParamMacroData& data);
JsonReader& operator&(JsonReader& self, ParamMacroData& data);
QByteArray& operator>>(QByteArray& self, ParamMacroData& data);
QByteArray& operator<<(QByteArray& self, const ParamMacroData& data);
QString& operator>>(QString& self, ParamMacroData& data);
QString& operator<<(QString& self, const ParamMacroData& data);

JsonWriter& operator&(JsonWriter& self, const ParamMacroDataList& data);
JsonReader& operator&(JsonReader& self, ParamMacroDataList& data);
QByteArray& operator>>(QByteArray& self, ParamMacroDataList& data);
QByteArray& operator<<(QByteArray& self, const ParamMacroDataList& data);
QString& operator>>(QString& self, ParamMacroDataList& data);
QString& operator<<(QString& self, const ParamMacroDataList& data);

JsonWriter& operator&(JsonWriter& self, const ParamMacroDataMap& data);
JsonReader& operator&(JsonReader& self, ParamMacroDataMap& data);
QByteArray& operator>>(QByteArray& self, ParamMacroDataMap& data);
QByteArray& operator<<(QByteArray& self, const ParamMacroDataMap& data);
QString& operator>>(QString& self, ParamMacroDataMap& data);
QString& operator<<(QString& self, const ParamMacroDataMap& data);

#endif  // DMMESSAGESERIALIZE_H
