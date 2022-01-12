#ifndef USERMANAGERSERVICE_H
#define USERMANAGERSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class UserManagerServiceImpl;
class UserManagerService : public INetMsg
{
    Q_OBJECT
public:
    UserManagerService(cppmicroservices::BundleContext context);
    ~UserManagerService();

private:
    void userRegisterACK(const ProtocolPack& pack);
    void deleteUserACK(const ProtocolPack& pack);
    void modifyUserACK(const ProtocolPack& pack);
    void userLoginACK(const ProtocolPack& pack);
    void userLogoutACK(const ProtocolPack& pack);
    void getUserListACK(const ProtocolPack& pack);

signals:
    void signalUserRegisterACK(const QByteArray& data);
    void signalDeleteUserACK(const QByteArray& data);
    void signalModifyUserACK(const QByteArray& data);
    void signalUserLoginACK(const QByteArray& data);
    void signalUserLogoutACK(const QByteArray& data);
    void signalGetUserListACK(const QByteArray& data);

public slots:
    void slotUserRegister(const QByteArray& data);
    void slotDeleteUser(const QByteArray& data);
    void slotModifyUser(const QByteArray& data);
    void slotUserLogin(const QByteArray& data);
    void slotUserLogout(const QByteArray& data);
    void slotGetUserList();

private:
    UserManagerServiceImpl* m_impl;
};
#endif  // USERMANAGERSERVICE_H
