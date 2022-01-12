#include "PhaseCalibrationService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "PhaseCalibration";
class PhaseCalibrationServiceImpl
{
public:
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
PhaseCalibrationService::PhaseCalibrationService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new PhaseCalibrationServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("startPhaseCalibrationACK", &PhaseCalibrationService::startPhaseCalibrationACK, this);
    registerSubscribe("stopPhaseCalibrationACK", &PhaseCalibrationService::stopPhaseCalibrationACK, this);
    registerSubscribe("deletePhaseCalibrationResultACK", &PhaseCalibrationService::deletePhaseCalibrationResultACK, this);
    registerSubscribe("clearHistryACK", &PhaseCalibrationService::clearHistryACK, this);
    registerSubscribe("queryHistryACK", &PhaseCalibrationService::queryHistryACK, this);
}

PhaseCalibrationService::~PhaseCalibrationService() { delete m_impl; }

void PhaseCalibrationService::startPhaseCalibration(const QByteArray& data)
{
    auto pack = PhaseCalibrationServiceImpl::packServerSelf("startPhaseCalibration", "startPhaseCalibrationACK", data);
    silenceSend(pack);
}
void PhaseCalibrationService::stopPhaseCalibration(const QByteArray& data)
{
    auto pack = PhaseCalibrationServiceImpl::packServerSelf("stopPhaseCalibration", "stopPhaseCalibrationACK", data);
    silenceSend(pack);
}
void PhaseCalibrationService::deletePhaseCalibrationResult(const QByteArray& data)
{
    auto pack = PhaseCalibrationServiceImpl::packServerSelf("deletePhaseCalibrationResult", "deletePhaseCalibrationResultACK", data);
    silenceSend(pack);
}
void PhaseCalibrationService::clearHistry(const QByteArray& data)
{
    auto pack = PhaseCalibrationServiceImpl::packServerSelf("clearHistry", "clearHistryACK", data);
    silenceSend(pack);
}

void PhaseCalibrationService::queryHistry(const QByteArray& data)
{
    auto pack = PhaseCalibrationServiceImpl::packServerSelf("queryHistry", "queryHistryACK", data);
    silenceSend(pack);
}

void PhaseCalibrationService::startPhaseCalibrationACK(const ProtocolPack& pack) { emit sg_startPhaseCalibrationACK(pack.data); }
void PhaseCalibrationService::stopPhaseCalibrationACK(const ProtocolPack& pack) { emit sg_stopPhaseCalibrationACK(pack.data); }
void PhaseCalibrationService::deletePhaseCalibrationResultACK(const ProtocolPack& pack) { emit sg_deletePhaseCalibrationResultACK(pack.data); }
void PhaseCalibrationService::clearHistryACK(const ProtocolPack& pack) { emit sg_clearHistryACK(pack.data); }
void PhaseCalibrationService::queryHistryACK(const ProtocolPack& pack) { emit sg_queryHistryACK(pack.data); }
