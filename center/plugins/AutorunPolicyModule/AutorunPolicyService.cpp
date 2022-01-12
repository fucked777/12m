#include "AutorunPolicyService.h"
#include "CppMicroServicesUtility.h"
#include "Utility.h"
#include <QMessageBox>

static constexpr auto serverSelfModule = "AutorunPolicy";
class AutorunPolicyServiceImpl
{
    DIS_COPY_MOVE(AutorunPolicyServiceImpl)
public:
    inline AutorunPolicyServiceImpl() {}
    inline ~AutorunPolicyServiceImpl() {}
    cppmicroservices::BundleContext context;
    /*
     * 组向服务器设备管理的包
     */
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
AutorunPolicyService::AutorunPolicyService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new AutorunPolicyServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("modifyItemACK", &AutorunPolicyService::modifyItemACK, this);
}

AutorunPolicyService::~AutorunPolicyService() { delete m_impl; }

void AutorunPolicyService::modifyItem(const QByteArray& data)
{
    auto pack = AutorunPolicyServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}
void AutorunPolicyService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
