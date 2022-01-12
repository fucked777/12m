#ifndef PLANRUNMESSAGESERIALIZE_H
#define PLANRUNMESSAGESERIALIZE_H

#include "PlanRunMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const TargetInfo& data);
JsonReader& operator&(JsonReader& self, TargetInfo& data);
QByteArray& operator>>(QByteArray& self, TargetInfo& data);
QByteArray& operator<<(QByteArray& self, const TargetInfo& data);
QString& operator>>(QString& self, TargetInfo& data);
QString& operator<<(QString& self, const TargetInfo& data);

JsonWriter& operator&(JsonWriter& self, const QMap<int, TargetInfo>& data);
JsonReader& operator&(JsonReader& self, QMap<int, TargetInfo>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, TargetInfo>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, TargetInfo>& data);
QString& operator>>(QString& self, QMap<int, TargetInfo>& data);
QString& operator<<(QString& self, const QMap<int, TargetInfo>& data);

JsonWriter& operator&(JsonWriter& self, const LinkLine& data);
JsonReader& operator&(JsonReader& self, LinkLine& data);
QByteArray& operator>>(QByteArray& self, LinkLine& data);
QByteArray& operator<<(QByteArray& self, const LinkLine& data);
QString& operator>>(QString& self, LinkLine& data);
QString& operator<<(QString& self, const LinkLine& data);

JsonWriter& operator&(JsonWriter& self, const QMap<SystemWorkMode, LinkLine>& data);
JsonReader& operator&(JsonReader& self, QMap<SystemWorkMode, LinkLine>& data);
QByteArray& operator>>(QByteArray& self, QMap<SystemWorkMode, LinkLine>& data);
QByteArray& operator<<(QByteArray& self, const QMap<SystemWorkMode, LinkLine>& data);
QString& operator>>(QString& self, QMap<SystemWorkMode, LinkLine>& data);
QString& operator<<(QString& self, const QMap<SystemWorkMode, LinkLine>& data);

JsonWriter& operator&(JsonWriter& self, const ManualMessage& data);
JsonReader& operator&(JsonReader& self, ManualMessage& data);
QByteArray& operator>>(QByteArray& self, ManualMessage& data);
QByteArray& operator<<(QByteArray& self, const ManualMessage& data);
QString& operator>>(QString& self, ManualMessage& data);
QString& operator<<(QString& self, const ManualMessage& data);

Q_DECL_CONST_FUNCTION Q_DECL_CONSTEXPR inline uint qHash(const LinkType& key, uint seed = 0) Q_DECL_NOTHROW
{
    return qHash(static_cast<int>(key), seed);
}
#endif  // PLANRUNMESSAGESERIALIZE_H
