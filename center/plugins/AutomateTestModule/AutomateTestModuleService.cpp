#include "AutomateTestModuleService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "AutomateTest";

class AutomateTestModuleServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    static inline ProtocolPack packServerSelf(const char* operation, const char* operationACK = "", const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = serverSelfModule;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = false;
        sendPack.data = data;
        return sendPack;
    }
};
AutomateTestModuleService::AutomateTestModuleService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new AutomateTestModuleServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addTestItemACK", &AutomateTestModuleService::addTestItemACK, this);
    registerSubscribe("addTestBindItemACK", &AutomateTestModuleService::addTestBindItemACK, this);
    registerSubscribe("deleteTestBindItemACK", &AutomateTestModuleService::deleteTestBindItemACK, this);
    registerSubscribe("deleteTestItemACK", &AutomateTestModuleService::deleteTestItemACK, this);
    registerSubscribe("deleteTestResultACK", &AutomateTestModuleService::deleteTestResultACK, this);
    registerSubscribe("startTestACK", &AutomateTestModuleService::startTestACK, this);
    registerSubscribe("stopTestACK", &AutomateTestModuleService::stopTestACK, this);
    registerSubscribe("queryTestItemACK", &AutomateTestModuleService::queryTestItemACK, this);
    registerSubscribe("queryTestResultACK", &AutomateTestModuleService::queryTestResultACK, this);
    registerSubscribe("clearHistryACK", &AutomateTestModuleService::clearHistryACK, this);
}

AutomateTestModuleService::~AutomateTestModuleService() { delete m_impl; }

void AutomateTestModuleService::addTestItem(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("addTestItem", "addTestItemACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::addTestBindItem(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("addTestBindItem", "addTestBindItemACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::deleteTestBindItem(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("deleteTestBindItem", "deleteTestBindItemACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::deleteTestItem(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("deleteTestItem", "deleteTestItemACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::deleteTestResult(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("deleteTestResult", "deleteTestResultACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::clearHistry(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("clearHistry", "clearHistryACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::startTest(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("startTest", "startTestACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::stopTest(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("stopTest", "stopTestACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::queryTestItem(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("queryTestItem", "queryTestItemACK", data);
    silenceSend(pack);
}
void AutomateTestModuleService::queryTestResult(const QByteArray& data)
{
    auto pack = AutomateTestModuleServiceImpl::packServerSelf("queryTestResult", "queryTestResultACK", data);
    silenceSend(pack);
}

void AutomateTestModuleService::addTestItemACK(const ProtocolPack& pack) { emit sg_addTestItemACK(pack.data); }
void AutomateTestModuleService::addTestBindItemACK(const ProtocolPack& pack) { emit sg_addTestBindItemACK(pack.data); }
void AutomateTestModuleService::deleteTestBindItemACK(const ProtocolPack& pack) { emit sg_deleteTestBindItemACK(pack.data); }
void AutomateTestModuleService::deleteTestItemACK(const ProtocolPack& pack) { emit sg_deleteTestItemACK(pack.data); }
void AutomateTestModuleService::deleteTestResultACK(const ProtocolPack& pack) { emit sg_deleteTestResultACK(pack.data); }
void AutomateTestModuleService::clearHistryACK(const ProtocolPack& pack) { emit sg_clearHistryACK(pack.data); }
void AutomateTestModuleService::startTestACK(const ProtocolPack& pack) { emit sg_startTestACK(pack.data); }
void AutomateTestModuleService::stopTestACK(const ProtocolPack& pack) { emit sg_stopTestACK(pack.data); }
void AutomateTestModuleService::queryTestItemACK(const ProtocolPack& pack) { emit sg_queryTestItemACK(pack.data); }
void AutomateTestModuleService::queryTestResultACK(const ProtocolPack& pack) { emit sg_queryTestResultACK(pack.data); }
