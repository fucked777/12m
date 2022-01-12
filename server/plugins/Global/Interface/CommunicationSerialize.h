#ifndef COMMUNICATIONSERIALIZE_H
#define COMMUNICATIONSERIALIZE_H
#include "CommunicationDefine.h"
#include "Utility.h"

#include <QByteArray>
#include <QDomElement>

class QDebug;

/* 失败QString为空,CommunicationType为None */
QString& operator<<(QString& self, CommunicationType value);
QString& operator>>(QString& self, CommunicationType& value);

JsonWriter& operator&(JsonWriter& self, const CommunicationInfo& value);
JsonReader& operator&(JsonReader& self, CommunicationInfo& value);
JsonWriter& operator&(JsonWriter& self, const CommunicationInfoMap& value);
JsonReader& operator&(JsonReader& self, CommunicationInfoMap& value);
JsonWriter& operator&(JsonWriter& self, const CommunicationInfoList& value);
JsonReader& operator&(JsonReader& self, CommunicationInfoList& value);

QByteArray& operator<<(QByteArray& self, const CommunicationInfo& value);
QByteArray& operator>>(QByteArray& self, CommunicationInfo& value);
QString& operator<<(QString& self, const CommunicationInfo& value);
QString& operator>>(QString& self, CommunicationInfo& value);

QByteArray& operator<<(QByteArray& self, const CommunicationInfoMap& value);
QByteArray& operator>>(QByteArray& self, CommunicationInfoMap& value);
QString& operator<<(QString& self, const CommunicationInfoMap& value);
QString& operator>>(QString& self, CommunicationInfoMap& value);

QByteArray& operator<<(QByteArray& self, const CommunicationInfoList& value);
QByteArray& operator>>(QByteArray& self, CommunicationInfoList& value);
QString& operator<<(QString& self, const CommunicationInfoList& value);
QString& operator>>(QString& self, CommunicationInfoList& value);

QDebug operator<<(QDebug self, const CommunicationInfo& value);

class CommunicationHelper
{
public:
    static QByteArray toByteArray(qint32 channel, const QByteArray& data);
    static void fromByteArray(qint32& channel, QByteArray& data);
    static void fromByteArray(const QByteArray& self, qint32& channel, QByteArray& data);
};

#endif  // COMMUNICATIONSERIALIZE_H
