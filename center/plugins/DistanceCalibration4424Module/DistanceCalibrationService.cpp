#include "DistanceCalibrationService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "DistanceCalibration";
class DistanceCalibrationServiceImpl
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
DistanceCalibrationService::DistanceCalibrationService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DistanceCalibrationServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("startDistanceCalibrationACK", &DistanceCalibrationService::startDistanceCalibrationACK, this);
    registerSubscribe("stopDistanceCalibrationACK", &DistanceCalibrationService::stopDistanceCalibrationACK, this);
    registerSubscribe("deleteDistanceCalibrationResultACK", &DistanceCalibrationService::deleteDistanceCalibrationResultACK, this);
    registerSubscribe("clearHistryACK", &DistanceCalibrationService::clearHistryACK, this);
    registerSubscribe("queryHistryACK", &DistanceCalibrationService::queryHistryACK, this);
}

DistanceCalibrationService::~DistanceCalibrationService() { delete m_impl; }

void DistanceCalibrationService::startDistanceCalibration(const QByteArray& data)
{
    auto pack = DistanceCalibrationServiceImpl::packServerSelf("startDistanceCalibration", "startDistanceCalibrationACK", data);
    silenceSend(pack);
}
void DistanceCalibrationService::stopDistanceCalibration(const QByteArray& data)
{
    auto pack = DistanceCalibrationServiceImpl::packServerSelf("stopDistanceCalibration", "stopDistanceCalibrationACK", data);
    silenceSend(pack);
}
void DistanceCalibrationService::deleteDistanceCalibrationResult(const QByteArray& data)
{
    auto pack = DistanceCalibrationServiceImpl::packServerSelf("deleteDistanceCalibrationResult", "deleteDistanceCalibrationResultACK", data);
    silenceSend(pack);
}
void DistanceCalibrationService::clearHistry(const QByteArray& data)
{
    auto pack = DistanceCalibrationServiceImpl::packServerSelf("clearHistry", "clearHistryACK", data);
    silenceSend(pack);
}

void DistanceCalibrationService::queryHistry(const QByteArray& data)
{
    auto pack = DistanceCalibrationServiceImpl::packServerSelf("queryHistry", "queryHistryACK", data);
    silenceSend(pack);
}

void DistanceCalibrationService::startDistanceCalibrationACK(const ProtocolPack& pack) { emit sg_startDistanceCalibrationACK(pack.data); }
void DistanceCalibrationService::stopDistanceCalibrationACK(const ProtocolPack& pack) { emit sg_stopDistanceCalibrationACK(pack.data); }
void DistanceCalibrationService::deleteDistanceCalibrationResultACK(const ProtocolPack& pack)
{
    emit sg_deleteDistanceCalibrationResultACK(pack.data);
}
void DistanceCalibrationService::clearHistryACK(const ProtocolPack& pack) { emit sg_clearHistryACK(pack.data); }
void DistanceCalibrationService::queryHistryACK(const ProtocolPack& pack) { emit sg_queryHistryACK(pack.data); }
