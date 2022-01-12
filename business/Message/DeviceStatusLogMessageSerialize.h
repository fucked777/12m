#ifndef DEVICESTATUSLOGMESSAGESERIALIZE_H
#define DEVICESTATUSLOGMESSAGESERIALIZE_H

#include "DeviceStatusLogMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const DeviceStatusRecord& value);
JsonReader& operator&(JsonReader& self, DeviceStatusRecord& value);
QByteArray& operator>>(QByteArray& self, DeviceStatusRecord& value);
QByteArray& operator<<(QByteArray& self, const DeviceStatusRecord& value);
QString& operator>>(QString& self, DeviceStatusRecord& value);
QString& operator<<(QString& self, const DeviceStatusRecord& value);

#endif  // DEVICESTATUSLOGMESSAGESERIALIZE_H
