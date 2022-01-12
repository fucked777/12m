#ifndef CUSTOMPACKETMESSAGESERIALIZE_H
#define CUSTOMPACKETMESSAGESERIALIZE_H

#include "CustomPacketMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const UnitReportMessage& data);
JsonReader& operator&(JsonReader& self, UnitReportMessage& data);
QByteArray& operator<<(QByteArray& self, const UnitReportMessage& data);
QByteArray& operator>>(QByteArray& self, UnitReportMessage& data);
QString& operator>>(QString& self, UnitReportMessage& data);
QString& operator<<(QString& self, const UnitReportMessage& data);

JsonWriter& operator&(JsonWriter& self, const ExtensionStatusReportMessage& data);
JsonReader& operator&(JsonReader& self, ExtensionStatusReportMessage& data);
QByteArray& operator<<(QByteArray& self, const ExtensionStatusReportMessage& data);
QByteArray& operator>>(QByteArray& self, ExtensionStatusReportMessage& data);
QString& operator>>(QString& self, ExtensionStatusReportMessage& data);
QString& operator<<(QString& self, const ExtensionStatusReportMessage& data);

#endif  // CUSTOMPACKETMESSAGESERIALIZE_H
