#include "UserManagerMessageSerialize.h"

JsonWriter& operator&(JsonWriter& self, const User& data)
{
    self.StartObject();

    self.Member("userID") & data.userID;
    self.Member("name") & data.name;
    self.Member("password") & data.password;
    self.Member("identity") & data.identity;
    self.Member("loginTime") & data.loginTime;
    self.Member("isOnline") & data.isOnline;

    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, User& data)
{
    self.StartObject();

    self.Member("userID") & data.userID;
    self.Member("name") & data.name;
    self.Member("password") & data.password;
    self.Member("identity") & data.identity;
    self.Member("loginTime") & data.loginTime;
    self.Member("isOnline") & data.isOnline;

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, User& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QByteArray& operator<<(QByteArray& self, const User& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, User& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const User& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const UserMap& data)
{
    self.StartObject();
    self.Member("User");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}

JsonReader& operator&(JsonReader& self, UserMap& data)
{
    self.StartObject();
    self.Member("User");
    std::size_t count = 0;

    self.StartArray(&count);
    User user;
    for (std::size_t i = 0; i < count; i++)
    {
        self& user;
        data.insert(user.userID, user);
    }
    self.EndArray();

    return self.EndObject();
}

QByteArray& operator>>(QByteArray& self, UserMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const UserMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, UserMap& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QString& operator<<(QString& self, const UserMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const UserList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}

JsonReader& operator&(JsonReader& self, UserList& data)
{
    std::size_t count = 0;

    self.StartArray(&count);
    User user;
    for (std::size_t i = 0; i < count; i++)
    {
        self& user;
        data.append(user);
    }

    return self.EndArray();
}

QByteArray& operator>>(QByteArray& self, UserList& data)
{
    JsonReader reader(self);
    reader& data;

    return self;
}

QByteArray& operator<<(QByteArray& self, const UserList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}

QString& operator>>(QString& self, UserList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}

QString& operator<<(QString& self, const UserList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
