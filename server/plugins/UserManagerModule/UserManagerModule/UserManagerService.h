#ifndef UserManagerSERVICE_H
#define UserManagerSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class UserManagerServiceImpl;
class UserManagerService : public INetMsg
{
public:
    UserManagerService(cppmicroservices::BundleContext context);
    ~UserManagerService();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void start();
    void stop();

    void userRegister(const ProtocolPack& pack);
    void deleteUser(const ProtocolPack& pack);
    void modifyUser(const ProtocolPack& pack);
    void userLogin(const ProtocolPack& pack);
    void userLogout(const ProtocolPack& pack);
    void getUserList(const ProtocolPack& pack);

private:
    UserManagerServiceImpl* mImpl;
};

#endif  // UserManagerSERVICE_H
