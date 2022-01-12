#include "ResourceRestructuringService.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "MessagePublish.h"
#include "ResourceRestructuringMessageSerialize.h"
#include "SqlResourceRestructuringManager.h"
#define MODULENAME "ResourceRestructuringModule"

class ResourceRestructuringServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlResourceRestructuringManager sql;
    int timerID{ -1 };

    static void pushData(const ResourceRestructuringDataList& data);
    Optional<ResourceRestructuringDataList> reload(bool push = true);
    void silenceReload(bool push = true);
};

ResourceRestructuringService::ResourceRestructuringService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new ResourceRestructuringServiceImpl)
{
    mImpl->context = context;

    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("addItem", &ResourceRestructuringService::addItem, this);
    registerSubscribe("deleteItem", &ResourceRestructuringService::deleteItem, this);
    registerSubscribe("queryItem", &ResourceRestructuringService::queryItem, this);
    registerSubscribe("modifyItem", &ResourceRestructuringService::modifyItem, this);

    mImpl->silenceReload();
    start();
}

ResourceRestructuringService::~ResourceRestructuringService()
{
    stop();
    delete mImpl;
}
void ResourceRestructuringService::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(50000);
    }
}
void ResourceRestructuringService::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

void ResourceRestructuringService::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void ResourceRestructuringService::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ResourceRestructuringData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加资源配置:%1成功").arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加资源配置:%1失败").arg(item.name), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ResourceRestructuringService::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ResourceRestructuringData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除资源配置:%1成功").arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除资源配置:%1失败").arg(item.name), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ResourceRestructuringService::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.selectItem();
    silenceSendACK(bak);
}

void ResourceRestructuringService::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ResourceRestructuringData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改资源配置:%1成功").arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改资源配置:%1失败").arg(item.name), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ResourceRestructuringServiceImpl::pushData(const ResourceRestructuringDataList& data) { GlobalData::setResourceRestructuringData(data); }

Optional<ResourceRestructuringDataList> ResourceRestructuringServiceImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        pushData(itemMap.value());
    }

    return itemMap;
}

void ResourceRestructuringServiceImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), msg);
        return;
    }
}
