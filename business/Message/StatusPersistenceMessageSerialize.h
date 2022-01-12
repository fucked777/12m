#ifndef STATUSPERSISTENCEMESSAGESERIALIZE_H
#define STATUSPERSISTENCEMESSAGESERIALIZE_H

#include "StatusPersistenceMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const ParamStatus& data);
JsonReader& operator&(JsonReader& self, ParamStatus& data);
QByteArray& operator>>(QByteArray& self, ParamStatus& data);
QByteArray& operator<<(QByteArray& self, const ParamStatus& data);
QString& operator>>(QString& self, ParamStatus& data);
QString& operator<<(QString& self, const ParamStatus& data);

JsonWriter& operator&(JsonWriter& self, const UnitStatusParam& data);
JsonReader& operator&(JsonReader& self, UnitStatusParam& data);
QByteArray& operator>>(QByteArray& self, UnitStatusParam& data);
QByteArray& operator<<(QByteArray& self, const UnitStatusParam& data);
QString& operator>>(QString& self, UnitStatusParam& data);
QString& operator<<(QString& self, const UnitStatusParam& data);

JsonWriter& operator&(JsonWriter& self, const UnitStatusParamMap& data);
JsonReader& operator&(JsonReader& self, UnitStatusParamMap& data);
QByteArray& operator>>(QByteArray& self, UnitStatusParamMap& data);
QByteArray& operator<<(QByteArray& self, const UnitStatusParamMap& data);
QString& operator>>(QString& self, UnitStatusParamMap& data);
QString& operator<<(QString& self, const UnitStatusParamMap& data);

JsonWriter& operator&(JsonWriter& self, const DeviceStatusData& data);
JsonReader& operator&(JsonReader& self, DeviceStatusData& data);
QByteArray& operator>>(QByteArray& self, DeviceStatusData& data);
QByteArray& operator<<(QByteArray& self, const DeviceStatusData& data);
QString& operator>>(QString& self, DeviceStatusData& data);
QString& operator<<(QString& self, const DeviceStatusData& data);

JsonWriter& operator&(JsonWriter& self, const QStringList& data);
JsonReader& operator&(JsonReader& self, QStringList& data);
QByteArray& operator>>(QByteArray& self, QStringList& data);
QByteArray& operator<<(QByteArray& self, const QStringList& data);
QString& operator>>(QString& self, QStringList& data);
QString& operator<<(QString& self, const QStringList& data);

JsonWriter& operator&(JsonWriter& self, const DeviceStatusDataCondition& data);
JsonReader& operator&(JsonReader& self, DeviceStatusDataCondition& data);
QByteArray& operator>>(QByteArray& self, DeviceStatusDataCondition& data);
QByteArray& operator<<(QByteArray& self, const DeviceStatusDataCondition& data);
QString& operator>>(QString& self, DeviceStatusDataCondition& data);
QString& operator<<(QString& self, const DeviceStatusDataCondition& data);

JsonWriter& operator&(JsonWriter& self, const DeviceStatusLogData& data);
JsonReader& operator&(JsonReader& self, DeviceStatusLogData& data);
QByteArray& operator>>(QByteArray& self, DeviceStatusLogData& data);
QByteArray& operator<<(QByteArray& self, const DeviceStatusLogData& data);
QString& operator>>(QString& self, DeviceStatusLogData& data);
QString& operator<<(QString& self, const DeviceStatusLogData& data);

JsonWriter& operator&(JsonWriter& self, const DeviceStatusLogDataList& data);
JsonReader& operator&(JsonReader& self, DeviceStatusLogDataList& data);
QByteArray& operator>>(QByteArray& self, DeviceStatusLogDataList& data);
QByteArray& operator<<(QByteArray& self, const DeviceStatusLogDataList& data);
QString& operator>>(QString& self, DeviceStatusLogDataList& data);
QString& operator<<(QString& self, const DeviceStatusLogDataList& data);

#endif  // STATUSPERSISTENCEMESSAGESERIALIZE_H
