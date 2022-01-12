#ifndef AUTORUNTASKLOGMESSAGESERIALIZE_H
#define AUTORUNTASKLOGMESSAGESERIALIZE_H

#include "AutoRunTaskLogMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const AutoTaskLogData& data);
JsonReader& operator&(JsonReader& self, AutoTaskLogData& data);
QByteArray& operator>>(QByteArray& self, AutoTaskLogData& data);
QByteArray& operator<<(QByteArray& self, const AutoTaskLogData& data);
QString& operator>>(QString& self, AutoTaskLogData& data);
QString& operator<<(QString& self, const AutoTaskLogData& data);

JsonWriter& operator&(JsonWriter& self, const AutoTaskLogDataList& data);
JsonReader& operator&(JsonReader& self, AutoTaskLogDataList& data);
QByteArray& operator>>(QByteArray& self, AutoTaskLogDataList& data);
QByteArray& operator<<(QByteArray& self, const AutoTaskLogDataList& data);
QString& operator>>(QString& self, AutoTaskLogDataList& data);
QString& operator<<(QString& self, const AutoTaskLogDataList& data);

JsonWriter& operator&(JsonWriter& self, const AutoTaskLogDataCondition& data);
JsonReader& operator&(JsonReader& self, AutoTaskLogDataCondition& data);
QByteArray& operator>>(QByteArray& self, AutoTaskLogDataCondition& data);
QByteArray& operator<<(QByteArray& self, const AutoTaskLogDataCondition& data);
QString& operator>>(QString& self, AutoTaskLogDataCondition& data);
QString& operator<<(QString& self, const AutoTaskLogDataCondition& data);

#endif  // AUTORUNTASKLOGMESSAGESERIALIZE_H
