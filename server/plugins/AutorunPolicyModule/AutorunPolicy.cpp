#include "AutorunPolicy.h"
#include "AutorunPolicyMessageSerialize.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlAutorunPolicy.h"
#include <QJsonDocument>
#include <QJsonObject>

class AutorunPolicyImpl
{
public:
    int timerID{ -1 };
    SqlAutorunPolicy sqlAutorunPolicy;
    cppmicroservices::BundleContext context;
    SqlAutorunPolicy sql;

    static void pushData(const AutorunPolicyData&);
    void silenceReload();
};
void AutorunPolicyImpl::pushData(const AutorunPolicyData& data) { GlobalData::setAutorunPolicyData(data); }
void AutorunPolicyImpl::silenceReload()
{
    auto tempData = sql.selectItem();
    if (tempData)
    {
        AutorunPolicyImpl::pushData(tempData.value());
        return;
    }

    auto msg = QString("查询自动化运行策略错误:%1").arg(tempData.msg());
    qWarning() << msg;
    //        Error(itemMap.errorCode(), msg);
}

AutorunPolicy::AutorunPolicy(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new AutorunPolicyImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("自动化运行策略初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("modifyItem", &AutorunPolicy::modifyItem, this);
    m_impl->silenceReload();
    start();
}

AutorunPolicy::~AutorunPolicy()
{
    stop();
    delete m_impl;
}

void AutorunPolicy::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void AutorunPolicy::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(10000);
    }
}
void AutorunPolicy::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}
void AutorunPolicy::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    AutorunPolicyData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("保存自动化运行策略成功"), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("保存自动化运行策略失败"), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}
