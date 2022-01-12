#include "TaskCenter.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlTaskCenter.h"
#include "TaskCenterSerialize.h"

class TaskCenterImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlTaskCenter sql;
    int timerID{ -1 };
    static void pushData(const TaskCenterMap&);
    Optional<TaskCenterMap> reload(bool push = true);

    void silenceReload(bool push = true);
};
void TaskCenterImpl::pushData(const TaskCenterMap& data) { GlobalData::setTaskCenterInfo(data); }
Optional<TaskCenterMap> TaskCenterImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        TaskCenterImpl::pushData(itemMap.value());
    }

    return itemMap;
}
void TaskCenterImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询任务中心数据错误:%1").arg(itemMap.msg());
        // Error(itemMap.errorCode(), itemMap.msg());
        return;
    }
}
TaskCenter::TaskCenter(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new TaskCenterImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("任务中心初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("reload", &TaskCenter::reload, this);
    registerSubscribe("refresh", &TaskCenter::refresh, this);
    registerSubscribe("addItem", &TaskCenter::addItem, this);
    registerSubscribe("deleteItem", &TaskCenter::deleteItem, this);
    registerSubscribe("queryItem", &TaskCenter::queryItem, this);
    registerSubscribe("modifyItem", &TaskCenter::modifyItem, this);

    m_impl->silenceReload();
    start();
}

TaskCenter::~TaskCenter()
{
    stop();
    delete m_impl;
}

void TaskCenter::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(75000);
    }
}
void TaskCenter::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}
void TaskCenter::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void TaskCenter::reload(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload();
    auto bak = pack;
    bak.data.clear();
    bak.data << itemMap;
    silenceSendACK(bak);
}
void TaskCenter::refresh(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload(false);
    auto bak = pack;
    bak.data.clear();
    bak.data << OptionalNotValue(itemMap.errorCode(), itemMap.msg());
    silenceSendACK(bak);
}
void TaskCenter::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加任务中心:%1成功").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加任务中心:%1失败").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}

void TaskCenter::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除任务中心:%1成功").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除任务中心:%1失败").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
void TaskCenter::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << m_impl->sql.selectItem();
    silenceSendACK(bak);
}
void TaskCenter::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改任务中心:%1成功").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改任务中心:%1失败").arg(item.centerCode), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
