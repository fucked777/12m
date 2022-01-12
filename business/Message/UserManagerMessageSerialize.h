#ifndef USERMANAGERMESSAGESERIALIZE_H
#define USERMANAGERMESSAGESERIALIZE_H

#include "UserManagerMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const User& data);
JsonReader& operator&(JsonReader& self, User& data);
QByteArray& operator>>(QByteArray& self, User& data);
QByteArray& operator<<(QByteArray& self, const User& data);
QString& operator>>(QString& self, User& data);
QString& operator<<(QString& self, const User& data);

JsonWriter& operator&(JsonWriter& self, const UserMap& data);
JsonReader& operator&(JsonReader& self, UserMap& data);
QByteArray& operator>>(QByteArray& self, UserMap& data);
QByteArray& operator<<(QByteArray& self, const UserMap& data);
QString& operator>>(QString& self, UserMap& data);
QString& operator<<(QString& self, const UserMap& data);

JsonWriter& operator&(JsonWriter& self, const UserList& data);
JsonReader& operator&(JsonReader& self, UserList& data);
QByteArray& operator>>(QByteArray& self, UserList& data);
QByteArray& operator<<(QByteArray& self, const UserList& data);
QString& operator>>(QString& self, UserList& data);
QString& operator<<(QString& self, const UserList& data);

#endif  // USERMANAGERMESSAGESERIALIZE_H
