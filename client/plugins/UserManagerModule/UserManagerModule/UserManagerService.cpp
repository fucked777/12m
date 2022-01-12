#include "UserManagerService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "ServerUserManagerService";

class UserManagerServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    static inline ProtocolPack packServerSelf(const char* operation, const char* operationACK = "", const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = serverSelfModule;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = false;
        sendPack.data = data;
        return sendPack;
    }
};
UserManagerService::UserManagerService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new UserManagerServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("userRegisterACK", &UserManagerService::userRegisterACK, this);
    registerSubscribe("deleteUserACK", &UserManagerService::deleteUserACK, this);
    registerSubscribe("modifyUserACK", &UserManagerService::modifyUserACK, this);
    registerSubscribe("userLoginACK", &UserManagerService::userLoginACK, this);
    registerSubscribe("userLogoutACK", &UserManagerService::userLogoutACK, this);
    registerSubscribe("getUserListACK", &UserManagerService::getUserListACK, this);
}

UserManagerService::~UserManagerService() { delete m_impl; }

void UserManagerService::userRegisterACK(const ProtocolPack& pack) { emit signalUserRegisterACK(pack.data); }

void UserManagerService::deleteUserACK(const ProtocolPack& pack) { emit signalDeleteUserACK(pack.data); }

void UserManagerService::modifyUserACK(const ProtocolPack& pack) { emit signalModifyUserACK(pack.data); }

void UserManagerService::userLoginACK(const ProtocolPack& pack) { emit signalUserLoginACK(pack.data); }

void UserManagerService::userLogoutACK(const ProtocolPack& pack) { emit signalUserLogoutACK(pack.data); }

void UserManagerService::getUserListACK(const ProtocolPack& pack) { emit signalGetUserListACK(pack.data); }

void UserManagerService::slotUserRegister(const QByteArray& data)
{
    auto pack = UserManagerServiceImpl::packServerSelf("userRegister", "userRegisterACK", data);
    silenceSend(pack);
}

void UserManagerService::slotDeleteUser(const QByteArray& data)
{
    auto pack = UserManagerServiceImpl::packServerSelf("deleteUser", "deleteUserACK", data);
    silenceSend(pack);
}

void UserManagerService::slotModifyUser(const QByteArray& data)
{
    auto pack = UserManagerServiceImpl::packServerSelf("modifyUser", "modifyUserACK", data);
    silenceSend(pack);
}

void UserManagerService::slotUserLogin(const QByteArray& data)
{
    auto pack = UserManagerServiceImpl::packServerSelf("userLogin", "userLoginACK", data);
    silenceSend(pack);
}

void UserManagerService::slotUserLogout(const QByteArray& data)
{
    auto pack = UserManagerServiceImpl::packServerSelf("userLogout", "userLogoutACK", data);
    silenceSend(pack);
}

void UserManagerService::slotGetUserList()
{
    auto pack = UserManagerServiceImpl::packServerSelf("getUserList", "getUserListACK");
    silenceSend(pack);
}
