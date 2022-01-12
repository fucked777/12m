#ifndef TIMEMESSAGESERIALIZE_H
#define TIMEMESSAGESERIALIZE_H

#include "TimeMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const TimeData& data);
JsonReader& operator&(JsonReader& self, TimeData& data);
QByteArray& operator>>(QByteArray& self, TimeData& data);
QByteArray& operator<<(QByteArray& self, const TimeData& data);
QString& operator>>(QString& self, TimeData& data);
QString& operator<<(QString& self, const TimeData& data);

JsonWriter& operator&(JsonWriter& self, const TimeConfigInfo& data);
JsonReader& operator&(JsonReader& self, TimeConfigInfo& data);
QByteArray& operator>>(QByteArray& self, TimeConfigInfo& data);
QByteArray& operator<<(QByteArray& self, const TimeConfigInfo& data);
QString& operator>>(QString& self, TimeConfigInfo& data);
QString& operator<<(QString& self, const TimeConfigInfo& data);

#endif  // TIMEMESSAGESERIALIZE_H
