#include "DeviceStatusLogService.h"
#include "CppMicroServicesUtility.h"

#include "DeviceStatusLogMessageSerialize.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "SqlDeviceStatusLogManager.h"

class DeviceStatusLogServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlDeviceStatusLogManager sql;
    int timerID{ -1 };

    static void pushData(const UserList& data);
    Optional<UserList> reload(bool push = true);
    void silenceReload(bool push = true);
};

DeviceStatusLogService::DeviceStatusLogService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new DeviceStatusLogServiceImpl)
{
    mImpl->context = context;

    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("用户管理初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("addItem", &DeviceStatusLogService::addItem, this);
    registerSubscribe("deleteItem", &DeviceStatusLogService::deleteItem, this);
    registerSubscribe("queryItem", &DeviceStatusLogService::queryItem, this);
    registerSubscribe("modifyItem", &DeviceStatusLogService::modifyItem, this);
    registerSubscribe("userLogin", &DeviceStatusLogService::userLogin, this);

    mImpl->silenceReload();
    start();
}

DeviceStatusLogService::~DeviceStatusLogService()
{
    stop();
    delete mImpl;
}

void DeviceStatusLogService::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(20000);
    }
}
void DeviceStatusLogService::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

void DeviceStatusLogService::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void DeviceStatusLogService::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    User item;
    bak.data >> item;
    bak.data.clear();

    bak.data << mImpl->sql.insertItem(item);
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void DeviceStatusLogService::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    User item;
    bak.data >> item;
    bak.data.clear();

    bak.data << mImpl->sql.deleteItem(item);
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void DeviceStatusLogService::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.selectItem();

    QString userInfo;
    userInfo << mImpl->sql.selectItem().value();

    //    silenceSendACK(bak);
}

void DeviceStatusLogService::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    User item;
    bak.data >> item;
    bak.data.clear();

    bak.data << mImpl->sql.updateItem(item);
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void DeviceStatusLogService::userLogin(const ProtocolPack& pack)
{
    auto bak = pack;
    User item;
    bak.data >> item;
    bak.data.clear();
    bak.data << mImpl->sql.userLogin(item);

    silenceSendACK(bak);
}

void DeviceStatusLogServiceImpl::pushData(const UserList& data)
{
    JsonWriter writer;
    writer& data;
    RedisHelper::getInstance().setData("UserInfo", writer.GetString());
}

Optional<UserList> DeviceStatusLogServiceImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        pushData(itemMap.value());
    }

    return itemMap;
}

void DeviceStatusLogServiceImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询用户管理数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), msg);
        return;
    }
}
