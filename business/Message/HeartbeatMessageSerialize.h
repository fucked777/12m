#ifndef HEARTBEATSERIALIZE_H
#define HEARTBEATSERIALIZE_H

#include "HeartbeatMessage.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const HeartbeatMessage& value);
JsonReader& operator&(JsonReader& self, HeartbeatMessage& value);

QByteArray& operator>>(QByteArray& self, HeartbeatMessage& value);
QByteArray& operator<<(QByteArray& self, const HeartbeatMessage& value);
QString& operator<<(QString& self, const HeartbeatMessage& value);
QString& operator>>(QString& self, HeartbeatMessage& value);

#endif  // HEARTBEATSERIALIZE_H
