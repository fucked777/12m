#ifndef LINKCONFIGMESSAGESERIALIZE_H
#define LINKCONFIGMESSAGESERIALIZE_H

#include "LinkConfigMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const Command& data);
JsonReader& operator&(JsonReader& self, Command& data);
QByteArray& operator>>(QByteArray& self, Command& data);
QByteArray& operator<<(QByteArray& self, const Command& data);
QString& operator>>(QString& self, Command& data);
QString& operator<<(QString& self, const Command& data);

JsonWriter& operator&(JsonWriter& self, const QList<Command>& data);
JsonReader& operator&(JsonReader& self, QList<Command>& data);
QByteArray& operator>>(QByteArray& self, QList<Command>& data);
QByteArray& operator<<(QByteArray& self, const QList<Command>& data);
QString& operator>>(QString& self, QList<Command>& data);
QString& operator<<(QString& self, const QList<Command>& data);

JsonWriter& operator&(JsonWriter& self, const QSet<QString>& data);
JsonReader& operator&(JsonReader& self, QSet<QString>& data);
QByteArray& operator>>(QByteArray& self, QSet<QString>& data);
QByteArray& operator<<(QByteArray& self, const QSet<QString>& data);
QString& operator>>(QString& self, QSet<QString>& data);
QString& operator<<(QString& self, const QSet<QString>& data);

JsonWriter& operator&(JsonWriter& self, const QSet<QSet<QString>>& data);
JsonReader& operator&(JsonReader& self, QSet<QSet<QString>>& data);
QByteArray& operator>>(QByteArray& self, QSet<QSet<QString>>& data);
QByteArray& operator<<(QByteArray& self, const QSet<QSet<QString>>& data);
QString& operator>>(QString& self, QSet<QSet<QString>>& data);
QString& operator<<(QString& self, const QSet<QSet<QString>>& data);

JsonWriter& operator&(JsonWriter& self, const CommandGroup& data);
JsonReader& operator&(JsonReader& self, CommandGroup& data);
QByteArray& operator>>(QByteArray& self, CommandGroup& data);
QByteArray& operator<<(QByteArray& self, const CommandGroup& data);
QString& operator>>(QString& self, CommandGroup& data);
QString& operator<<(QString& self, const CommandGroup& data);

JsonWriter& operator&(JsonWriter& self, const QList<CommandGroup>& data);
JsonReader& operator&(JsonReader& self, QList<CommandGroup>& data);
QByteArray& operator>>(QByteArray& self, QList<CommandGroup>& data);
QByteArray& operator<<(QByteArray& self, const QList<CommandGroup>& data);
QString& operator>>(QString& self, QList<CommandGroup>& data);
QString& operator<<(QString& self, const QList<CommandGroup>& data);

JsonWriter& operator&(JsonWriter& self, const LinkMode& data);
JsonReader& operator&(JsonReader& self, LinkMode& data);
QByteArray& operator>>(QByteArray& self, LinkMode& data);
QByteArray& operator<<(QByteArray& self, const LinkMode& data);
QString& operator>>(QString& self, LinkMode& data);
QString& operator<<(QString& self, const LinkMode& data);

JsonWriter& operator&(JsonWriter& self, const QMap<int, LinkMode>& data);
JsonReader& operator&(JsonReader& self, QMap<int, LinkMode>& data);
QByteArray& operator>>(QByteArray& self, QMap<int, LinkMode>& data);
QByteArray& operator<<(QByteArray& self, const QMap<int, LinkMode>& data);
QString& operator>>(QString& self, QMap<int, LinkMode>& data);
QString& operator<<(QString& self, const QMap<int, LinkMode>& data);

JsonWriter& operator&(JsonWriter& self, const LinkConfig& data);
JsonReader& operator&(JsonReader& self, LinkConfig& data);
QByteArray& operator>>(QByteArray& self, LinkConfig& data);
QByteArray& operator<<(QByteArray& self, const LinkConfig& data);
QString& operator>>(QString& self, LinkConfig& data);
QString& operator<<(QString& self, const LinkConfig& data);

JsonWriter& operator&(JsonWriter& self, const LinkConfigMap& data);
JsonReader& operator&(JsonReader& self, LinkConfigMap& data);
QByteArray& operator>>(QByteArray& self, LinkConfigMap& data);
QByteArray& operator<<(QByteArray& self, const LinkConfigMap& data);
QString& operator>>(QString& self, LinkConfigMap& data);
QString& operator<<(QString& self, const LinkConfigMap& data);

#endif  // LINKCONFIGMESSAGESERIALIZE_H
