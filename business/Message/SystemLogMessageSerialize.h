#ifndef SYSTEMLOGMESSAGESERIALIZE_H
#define SYSTEMLOGMESSAGESERIALIZE_H

#include "SystemLogMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const SystemLogData& data);
JsonReader& operator&(JsonReader& self, SystemLogData& data);
QByteArray& operator>>(QByteArray& self, SystemLogData& data);
QByteArray& operator<<(QByteArray& self, const SystemLogData& data);
QString& operator>>(QString& self, SystemLogData& data);
QString& operator<<(QString& self, const SystemLogData& data);

JsonWriter& operator&(JsonWriter& self, const QList<SystemLogData>& data);
JsonReader& operator&(JsonReader& self, QList<SystemLogData>& data);
QByteArray& operator>>(QByteArray& self, QList<SystemLogData>& data);
QByteArray& operator<<(QByteArray& self, const QList<SystemLogData>& data);
QString& operator>>(QString& self, QList<SystemLogData>& data);
QString& operator<<(QString& self, const QList<SystemLogData>& data);

JsonWriter& operator&(JsonWriter& self, const SystemLogCondition& data);
JsonReader& operator&(JsonReader& self, SystemLogCondition& data);
QByteArray& operator>>(QByteArray& self, SystemLogCondition& data);
QByteArray& operator<<(QByteArray& self, const SystemLogCondition& data);
QString& operator>>(QString& self, SystemLogCondition& data);
QString& operator<<(QString& self, const SystemLogCondition& data);

JsonWriter& operator&(JsonWriter& self, const SystemLogACK& data);
JsonReader& operator&(JsonReader& self, SystemLogACK& data);
QByteArray& operator>>(QByteArray& self, SystemLogACK& data);
QByteArray& operator<<(QByteArray& self, const SystemLogACK& data);
QString& operator>>(QString& self, SystemLogACK& data);
QString& operator<<(QString& self, const SystemLogACK& data);


class SystemLogHelper
{
public:
    static QString logLevelToString(const LogLevel& level);
    static QColor logLevelToColor(const LogLevel& level);
    static QString logTypeString(const LogType& type);
};

#endif  // SYSTEMLOGMESSAGESERIALIZE_H
