#ifndef SERVICEINFODEFINESERIALIZE_H
#define SERVICEINFODEFINESERIALIZE_H

#include "ServiceInfoDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const ServiceInfo& value);
JsonReader& operator&(JsonReader& self, ServiceInfo& value);

QByteArray& operator>>(QByteArray& self, ServiceInfo& value);
QByteArray& operator<<(QByteArray& self, const ServiceInfo& value);
QString& operator<<(QString& self, const ServiceInfo& value);
QString& operator>>(QString& self, ServiceInfo& value);

#endif  // SERVICEINFODEFINESERIALIZE_H
