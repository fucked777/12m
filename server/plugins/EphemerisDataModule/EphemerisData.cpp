#include "EphemerisData.h"
#include "CppMicroServicesUtility.h"
#include "EphemerisDataSerialize.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlEphemerisData.h"

class EphemerisDataImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlEphemerisData sql;
    int timerID{ -1 };
    static void pushData(const EphemerisDataMap&);
    Optional<EphemerisDataMap> reload(bool push = true);

    void silenceReload(bool push = true);
};
void EphemerisDataImpl::pushData(const EphemerisDataMap& data) { GlobalData::setEphemerisDataInfo(data); }
Optional<EphemerisDataMap> EphemerisDataImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        EphemerisDataImpl::pushData(itemMap.value());
    }

    return itemMap;
}
void EphemerisDataImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询星历数据数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), msg);
        return;
    }
}

EphemerisData::EphemerisData(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new EphemerisDataImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("星历数据初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("reload", &EphemerisData::reload, this);
    registerSubscribe("refresh", &EphemerisData::refresh, this);
    registerSubscribe("addItem", &EphemerisData::addItem, this);
    registerSubscribe("deleteItem", &EphemerisData::deleteItem, this);
    registerSubscribe("queryItem", &EphemerisData::queryItem, this);
    registerSubscribe("modifyItem", &EphemerisData::modifyItem, this);

    m_impl->silenceReload();
    start();
}

EphemerisData::~EphemerisData()
{
    stop();
    delete m_impl;
}
void EphemerisData::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(70000);
    }
}
void EphemerisData::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}

void EphemerisData::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void EphemerisData::reload(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload();
    auto bak = pack;
    bak.data.clear();
    bak.data << itemMap;
    silenceSendACK(bak);
}
void EphemerisData::refresh(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload(false);
    auto bak = pack;
    bak.data.clear();
    bak.data << OptionalNotValue(itemMap.errorCode(), itemMap.msg());
    silenceSendACK(bak);
}
void EphemerisData::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    EphemerisDataData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加星历:%1成功").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加星历:%1失败").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);

    m_impl->silenceReload();
}

void EphemerisData::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    EphemerisDataData item;
    bak.data >> item;
    bak.data.clear();
    auto ans = m_impl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除星历:%1成功").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除星历:%1失败").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
void EphemerisData::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << m_impl->sql.selectItem();
    silenceSendACK(bak);
}
void EphemerisData::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    EphemerisDataData item;
    bak.data >> item;
    bak.data.clear();
    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改星历:%1成功").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改星历:%1失败").arg(item.satellitCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
