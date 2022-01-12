#ifndef USERMANAGERMESSAGEDEFINE_H
#define USERMANAGERMESSAGEDEFINE_H

#include <QMap>
#include <QString>
#include <QDataStream>

struct User
{
    QString userID;     // 用户唯一ID
    QString name;       // 用户名称
    QString password;   // 用户密码
    QString identity;   // 用户权限
    QString loginTime;  // 上次登录时间
    QString isOnline;   // 是否在线

    User()
        : userID("")
        , name("")
        , password("")
        , identity("")
        , loginTime("")
        , isOnline("")
    {
    }
};

using UserMap = QMap<QString, User>;
using UserList = QList<User>;

// 用户登录成功后，向服务器发送心跳
struct UserOnlineHeartbeat
{
public:
    QString userID;
    QString username;

    friend QDataStream& operator<<(QDataStream& dt, const UserOnlineHeartbeat& t)
    {
        dt << t.userID << t.username;
        return dt;
    }

    friend QDataStream& operator>>(QDataStream& dt, UserOnlineHeartbeat& t)
    {
        dt >> t.userID >> t.username;
        return dt;
    }
    QByteArray toByteArray()
    {
        QByteArray array;
        QDataStream stream(&array, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << *this;
        return array;
    }
    void fromByteArray(QByteArray array)
    {
        QDataStream stream(&array, QIODevice::ReadWrite);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream >> *this;
    }
};

#endif  // USERMANAGERMESSAGEDEFINE_H
