#include "UserManagerService.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlUserManagerManager.h"
#include "UserManagerMessageSerialize.h"
#include <QUuid>
#define MODULENAME "UserManagerModule"

class UserManagerServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlUserManagerManager sql;
    int timerID{ -1 };

    static void pushData(const UserList& data);
    Optional<UserList> reload(bool push = true);
    void silenceReload(bool push = true);
};

UserManagerService::UserManagerService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new UserManagerServiceImpl)
{
    mImpl->context = context;

    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("用户管理初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("userRegister", &UserManagerService::userRegister, this);
    registerSubscribe("deleteUser", &UserManagerService::deleteUser, this);
    registerSubscribe("modifyUser", &UserManagerService::modifyUser, this);
    registerSubscribe("userLogin", &UserManagerService::userLogin, this);
    registerSubscribe("userLogout", &UserManagerService::userLogout, this);
    registerSubscribe("getUserList", &UserManagerService::getUserList, this);

    mImpl->silenceReload();

    User user;
    user.userID = QUuid::createUuid().toString();
    user.name = "root";
    user.password = "123456";
    user.identity = "2";
    user.loginTime = GlobalData::currentDateTime().toString("yyyyMMddHHmmss");
    user.isOnline = "0";
    Optional<bool> result = mImpl->sql.defaultUserRegister(user);
    if (!result.success())
    {
        qWarning() << "用户管理默认root用户插入失败";
    }
    start();
}

UserManagerService::~UserManagerService()
{
    stop();
    delete mImpl;
}

void UserManagerService::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(46000);
    }
}
void UserManagerService::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

void UserManagerService::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void UserManagerService::userRegister(const ProtocolPack& pack)
{
    auto bak = pack;
    User user;
    bak.data >> user;
    bak.data.clear();

    auto ans = mImpl->sql.userRegister(user);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("注册用户%1成功").arg(user.name), bak.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("注册用户%1失败").arg(user.name), bak.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void UserManagerService::deleteUser(const ProtocolPack& pack)
{
    auto bak = pack;
    User user;
    bak.data >> user;
    bak.data.clear();

    auto ans = mImpl->sql.deleteUser(user);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除用户%1成功").arg(user.name), bak.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除用户%1失败").arg(user.name), bak.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void UserManagerService::getUserList(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.getUserList();

    QString userInfo;
    userInfo << mImpl->sql.getUserList().value();

    silenceSendACK(bak);
}

void UserManagerService::modifyUser(const ProtocolPack& pack)
{
    auto bak = pack;
    User user;
    bak.data >> user;
    bak.data.clear();

    auto ans = mImpl->sql.modifyUser(user);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改用户%1成功").arg(user.name), bak.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改用户%1失败").arg(user.name), bak.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void UserManagerService::userLogin(const ProtocolPack& pack)
{
    auto bak = pack;
    User user;
    bak.data >> user;
    bak.data.clear();

    auto ans = mImpl->sql.userLogin(user);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("用户%1登录成功").arg(user.name), user.name, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("用户%1登录失败").arg(user.name), user.name, MODULENAME);
    }
    bak.data << ans;

    silenceSendACK(bak);
}

void UserManagerService::userLogout(const ProtocolPack& pack)
{
    auto bak = pack;
    User user;
    bak.data >> user;
    bak.data.clear();
    bak.data << mImpl->sql.userLogout(user);

    silenceSendACK(bak);
}

void UserManagerServiceImpl::pushData(const UserList& data)
{
    JsonWriter writer;
    writer& data;
    RedisHelper::getInstance().setData("UserInfo", writer.GetString());
}

Optional<UserList> UserManagerServiceImpl::reload(bool push)
{
    auto userlist = sql.getUserList();
    if (userlist.success() & push)
    {
        pushData(userlist.value());
    }

    return userlist;
}

void UserManagerServiceImpl::silenceReload(bool push)
{
    auto userlist = reload(push);
    if (!userlist)
    {
        auto msg = QString("查询用户管理数据错误:%1").arg(userlist.msg());
        // Error(itemMap.errorCode(), msg);
        return;
    }
}
