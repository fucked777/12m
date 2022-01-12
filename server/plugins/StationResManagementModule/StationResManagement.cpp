#include "StationResManagement.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlStationResManagement.h"
#include "StationResManagementSerialize.h"

class StationResManagementImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlStationResManagement sql;
    int timerID{ -1 };
    static void pushData(const StationResManagementMap&);
    Optional<StationResManagementMap> reload(bool push = true);

    void silenceReload(bool push = true);
};
void StationResManagementImpl::pushData(const StationResManagementMap& data) { GlobalData::setStationResManagementInfo(data); }
Optional<StationResManagementMap> StationResManagementImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        StationResManagementImpl::pushData(itemMap.value());
    }

    return itemMap;
}
void StationResManagementImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询站资源数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), itemMap.msg());
        return;
    }
}
StationResManagement::StationResManagement(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new StationResManagementImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("任务中心初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("reload", &StationResManagement::reload, this);
    registerSubscribe("refresh", &StationResManagement::refresh, this);
    registerSubscribe("addItem", &StationResManagement::addItem, this);
    registerSubscribe("deleteItem", &StationResManagement::deleteItem, this);
    registerSubscribe("queryItem", &StationResManagement::queryItem, this);
    registerSubscribe("modifyItem", &StationResManagement::modifyItem, this);

    m_impl->silenceReload();
    start();
}

StationResManagement::~StationResManagement()
{
    stop();
    delete m_impl;
}
void StationResManagement::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(30000);
    }
}
void StationResManagement::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}

void StationResManagement::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void StationResManagement::reload(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload();
    auto bak = pack;
    bak.data.clear();
    bak.data << itemMap;
    silenceSendACK(bak);
}
void StationResManagement::refresh(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload(false);
    auto bak = pack;
    bak.data.clear();
    bak.data << OptionalNotValue(itemMap.errorCode(), itemMap.msg());
    silenceSendACK(bak);
}
void StationResManagement::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    StationResManagementData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加站信息:%1成功").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加站信息:%1失败").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);

    m_impl->silenceReload();
}

void StationResManagement::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    StationResManagementData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除站信息:%1成功").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除站信息:%1失败").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);

    m_impl->silenceReload();
}
void StationResManagement::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << m_impl->sql.selectItem();
    silenceSendACK(bak);
    // m_impl->silenceReload();
}
void StationResManagement::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    StationResManagementData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改站信息:%1成功").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改站信息:%1失败").arg(item.stationName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);

    m_impl->silenceReload();
}
