#ifndef SQLUserManagerMANAGER_H
#define SQLUserManagerMANAGER_H

#include "Utility.h"
#include <QObject>

struct User;
using UserList = QList<User>;
class SqlUserManagerManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlUserManagerManager(QObject* parent = nullptr);
    ~SqlUserManagerManager();

public:
    OptionalNotValue initDB();

    Optional<User> userRegister(const User& user);
    Optional<User> deleteUser(const User& user);
    Optional<User> modifyUser(const User& user);
    Optional<User> userLogin(const User& user);
    Optional<User> userLogout(const User& user);
    Optional<UserList> getUserList();
    Optional<bool> defaultUserRegister(const User& user);

private:
    QString mDbConnectName;
};

#endif  // SQLUserManagerMANAGER_H
