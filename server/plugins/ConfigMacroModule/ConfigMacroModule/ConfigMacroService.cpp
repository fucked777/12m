#include "ConfigMacroService.h"
#include "CppMicroServicesUtility.h"

#include "ConfigMacroMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "SqlConfigMacroManager.h"

#define MODULENAME "ConfigMacroModule"

class ConfigMacroServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlConfigMacroManager sql;
    int timerID{ -1 };

    static void pushData(const ConfigMacroDataList& data);
    Optional<ConfigMacroDataList> reload(bool push = true);
    void silenceReload(bool push = true);
};

ConfigMacroService::ConfigMacroService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new ConfigMacroServiceImpl)
{
    mImpl->context = context;

    // 初始化数据库
    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("配置宏初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("addItem", &ConfigMacroService::addItem, this);
    registerSubscribe("deleteItem", &ConfigMacroService::deleteItem, this);
    registerSubscribe("queryItem", &ConfigMacroService::queryItem, this);
    registerSubscribe("modifyItem", &ConfigMacroService::modifyItem, this);

    mImpl->silenceReload();
    start();
}

void ConfigMacroService::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(60000);
    }
}

void ConfigMacroService::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

ConfigMacroService::~ConfigMacroService()
{
    stop();
    delete mImpl;
}

void ConfigMacroService::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void ConfigMacroService::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ConfigMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加配置宏:%1模式成功").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加配置宏:%1模式失败").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ConfigMacroService::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ConfigMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除配置宏:%1模式成功").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除配置宏:%1模式失败").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ConfigMacroService::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.selectItem();
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ConfigMacroService::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ConfigMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改配置宏:%1模式成功").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改配置宏:%1模式失败").arg(item.workModeDesc), pack.userID, MODULENAME);
    }
    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ConfigMacroServiceImpl::pushData(const ConfigMacroDataList& data) { GlobalData::setConfigMacroData(data); }

Optional<ConfigMacroDataList> ConfigMacroServiceImpl::reload(bool push)
{
    auto items = sql.selectItem();
    if (items.success() & push)
    {
        pushData(items.value());
    }

    return items;
}

void ConfigMacroServiceImpl::silenceReload(bool push)
{
    auto items = reload(push);
    if (!items)
    {
        auto msg = QString("查询配置宏数据错误:%1").arg(items.msg());
        SystemLogPublish::errorMsg(msg);
        return;
    }
}
