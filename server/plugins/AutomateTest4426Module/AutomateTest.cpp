#include "AutomateTest.h"
#include "AutomateTestBase.h"
#include "AutomateTestFactory.h"
#include "AutomateTestSerialize.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlAutomateTest.h"
#include <QString>
#include <QThread>

class AutomateTestImpl
{
public:
    cppmicroservices::BundleContext context;
    AutomateTestBase* automateTestBase{ nullptr };
    int timerID{ -1 };

    void silenceReload();
    void silenceTestPlanReload();
    void silenceTestBindReload();
};

void AutomateTestImpl::silenceReload()
{
    silenceTestPlanReload();
    silenceTestBindReload();
}
void AutomateTestImpl::silenceTestPlanReload()
{
    auto result = SqlAutomateTest::queryTestItem();
    if (result)
    {
        GlobalData::setAutomateTestPlanItemList(result.value());
    }
}
void AutomateTestImpl::silenceTestBindReload()
{
    auto result = SqlAutomateTest::queryTestBind();
    if (result)
    {
        GlobalData::setAutomateTestParameterBindMap(result.value());
    }
}

AutomateTest::AutomateTest(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new AutomateTestImpl)
{
    m_impl->context = context;

    auto initDBRes = SqlAutomateTest::initDB();
    if (!initDBRes)
    {
        auto msg = QString("自动化测试初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("addTestItem", &AutomateTest::addTestItem, this);
    registerSubscribe("deleteTestItem", &AutomateTest::deleteTestItem, this);
    registerSubscribe("clearHistry", &AutomateTest::clearHistry, this);
    registerSubscribe("deleteTestResult", &AutomateTest::deleteTestResult, this);
    registerSubscribe("startTest", &AutomateTest::startTest, this);
    registerSubscribe("stopTest", &AutomateTest::stopTest, this);
    registerSubscribe("queryTestItem", &AutomateTest::queryTestItem, this);
    registerSubscribe("queryTestResult", &AutomateTest::queryTestResult, this);
    registerSubscribe("addTestBindItem", &AutomateTest::addTestBindItem, this);
    registerSubscribe("deleteTestBindItem", &AutomateTest::deleteTestBindItem, this);

    registerSubscribe("AcceptDeviceCMDResultMessage", &AutomateTest::sg_deviceCMDResult, this);
    registerSubscribe("AcceptDeviceCMDResponceMessage", &AutomateTest::sg_deviceCMDResponce, this);

    m_impl->silenceReload();
    start();
}

AutomateTest::~AutomateTest()
{
    stopTestPlan();
    delete m_impl;
}
void AutomateTest::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void AutomateTest::stopTestPlan()
{
    auto bak = m_impl->automateTestBase;
    m_impl->automateTestBase = nullptr;
    AutomateTestFactory::destroyAutomateTestItem(bak);
}

void AutomateTest::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(20000);
    }
}
void AutomateTest::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}

void AutomateTest::addTestItem(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestPlanItemList items;
    bak.data >> items;
    bak.data.clear();

    auto ans = SqlAutomateTest::insertTestItem(items);
    if (ans.success() && !items.isEmpty())
    {
        SystemLogPublish::infoMsg(QString("新增测试计划:%1成功").arg(items.first().testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    else if (!items.isEmpty())
    {
        SystemLogPublish::errorMsg(QString("新增测试计划:%1失败").arg(items.first().testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("新增测试计划失败"), bak.userID, MACRSTR(MODULE_NAME));
    }

    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}
void AutomateTest::deleteTestItem(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestPlanItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = SqlAutomateTest::deleteTestItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除测试计划:%1成功").arg(item.testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除测试计划:%1失败").arg(item.testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}

void AutomateTest::clearHistry(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestPlanItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = SqlAutomateTest::deleteHistoryTestItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除测试计划:%1成功").arg(item.testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除测试计划:%1失败").arg(item.testName), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}
void AutomateTest::addTestBindItem(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestParameterBind item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlAutomateTest::updateTestBind(item);
    m_impl->silenceReload();
    silenceSendACK(bak);
}
void AutomateTest::deleteTestBindItem(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestParameterBind item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlAutomateTest::deleteTestBind(item);
    m_impl->silenceReload();
    silenceSendACK(bak);
}

void AutomateTest::startTest(const ProtocolPack& pack)
{
    auto bak = pack;
    if (m_impl->automateTestBase != nullptr)
    {
        TestProcessACK statusACK;
        statusACK.status = AutomateTestStatus::Busy;
        statusACK.msg = QString("当前已有测试正在进行:%1").arg(m_impl->automateTestBase->curtestTypeName());
        bak.data.clear();
        bak.data << statusACK;
        silenceSendACK(bak);
        return;
    }

    AutomateTestPlanItem tempItem;
    bak.data >> tempItem;

    m_impl->automateTestBase = AutomateTestFactory::createAutomateTestItem(tempItem.testTypeID, tempItem.workMode, this);
    if (m_impl->automateTestBase == nullptr)
    {
        TestProcessACK statusACK;
        statusACK.status = AutomateTestStatus::Failed;
        statusACK.msg = QString("当前无法进行此项测试");
        bak.data.clear();
        bak.data << statusACK;
        silenceSendACK(bak);
        return;
    }
    connect(m_impl->automateTestBase, &AutomateTestBase::sg_end, this, &AutomateTest::stopTestPlan, Qt::ConnectionType::QueuedConnection);
    m_impl->automateTestBase->startTest(pack);
}
void AutomateTest::stopTest(const ProtocolPack& pack)
{
    stopTestPlan();
    auto bak = pack;
    bak.data.clear();
    silenceSendACK(bak);
}

void AutomateTest::deleteTestResult(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestResult item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlAutomateTest::deleteTestResultItem(item);
    m_impl->silenceReload();
    silenceSendACK(bak);
}
void AutomateTest::queryTestResult(const ProtocolPack& pack)
{
    auto bak = pack;
    AutomateTestResultQuery item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlAutomateTest::queryTestResultItem(item);
    m_impl->silenceReload();
    silenceSendACK(bak);
}

void AutomateTest::queryTestItem(const ProtocolPack& /*pack*/) {}
