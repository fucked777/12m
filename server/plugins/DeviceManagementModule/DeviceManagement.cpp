#include "DeviceManagement.h"
#include "CppMicroServicesUtility.h"
#include "DMMessageSerialize.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlDeviceManagement.h"
#include "Utility.h"
#include <array>
#include <mutex>

class DeviceManagementImpl
{
public:
    SqlDeviceManagement sql;
    cppmicroservices::BundleContext context;

    int timerID{ -1 };

    static void pushData(const DMTypeMap&);
    Optional<DMTypeMap> reload(bool push = true);

    void silenceReload(bool push = true);
};
void DeviceManagementImpl::pushData(const DMTypeMap& data) { GlobalData::setDeviceManagerInfo(data); }
Optional<DMTypeMap> DeviceManagementImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        DeviceManagementImpl::pushData(itemMap.value());
    }

    return itemMap;
}
void DeviceManagementImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询设备管理数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), msg);
        return;
    }
}
DeviceManagement::DeviceManagement(cppmicroservices::BundleContext context)
    : m_impl(new DeviceManagementImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("设备管理初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("reload", &DeviceManagement::reload, this);
    registerSubscribe("refresh", &DeviceManagement::refresh, this);
    registerSubscribe("addItem", &DeviceManagement::addItem, this);
    registerSubscribe("deleteItem", &DeviceManagement::deleteItem, this);
    registerSubscribe("queryItem", &DeviceManagement::queryItem, this);
    registerSubscribe("modifyItem", &DeviceManagement::modifyItem, this);

    m_impl->silenceReload(true);
    start();
}

DeviceManagement::~DeviceManagement()
{
    stop();
    delete m_impl;
}
void DeviceManagement::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(11000);
    }
}
void DeviceManagement::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}
void DeviceManagement::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void DeviceManagement::reload(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload();
    auto bak = pack;
    bak.data.clear();
    bak.data << itemMap;
    silenceSendACK(bak);
}
void DeviceManagement::refresh(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload(false);
    auto bak = pack;
    bak.data.clear();
    bak.data << OptionalNotValue(itemMap.errorCode(), itemMap.msg());
    silenceSendACK(bak);
}
void DeviceManagement::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DMDataItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加%1成功参数").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加%1失败参数").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}

void DeviceManagement::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DMDataItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除%1参数成功").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除%1参数失败").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
void DeviceManagement::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DMQuery item;
    bak.data >> item;
    bak.data.clear();

    bak.data << m_impl->sql.selectItem(item);
    silenceSendACK(bak);
}
void DeviceManagement::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DMDataItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改%1参数成功").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改%1参数失败").arg(item.typeName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
