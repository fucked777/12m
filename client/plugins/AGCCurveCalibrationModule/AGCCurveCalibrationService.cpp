#include "AGCCurveCalibrationService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "AGCCurveCalibration";
class AGCCurveCalibrationServiceImpl
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
AGCCurveCalibrationService::AGCCurveCalibrationService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new AGCCurveCalibrationServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("selectAGCDataACK", &AGCCurveCalibrationService::selectAGCDataACK, this);
    registerSubscribe("saveAGCDataACK", &AGCCurveCalibrationService::saveAGCDataACK, this);
    registerSubscribe("startAGCDataACK", &AGCCurveCalibrationService::startAGCDataACK, this);
    registerSubscribe("endAGCDataACK", &AGCCurveCalibrationService::endAGCDataACK, this);
    registerSubscribe("deleteAGCDataACK", &AGCCurveCalibrationService::deleteAGCDataACK, this);
    registerSubscribe("finishingItemACK", &AGCCurveCalibrationService::finishingItemACK, this);
    registerSubscribe("sendToMACBACK", &AGCCurveCalibrationService::sendToMACBACK, this);
    registerSubscribe("sendToACUACK", &AGCCurveCalibrationService::sendToACUACK, this);
}

AGCCurveCalibrationService::~AGCCurveCalibrationService() { delete m_impl; }

void AGCCurveCalibrationService::selectAGCData(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("selectAGCData", "selectAGCDataACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::saveAGCData(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("saveAGCData", "saveAGCDataACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::startAGCData(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("startAGCData", "startAGCDataACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::endAGCData(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("endAGCData", "endAGCDataACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::deleteAGCData(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("deleteAGCData", "deleteAGCDataACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::finishingItem(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("finishingItem", "finishingItemACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::sendToMACB(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("sendToMACB", "sendToMACBACK", data);
    silenceSend(pack);
}
void AGCCurveCalibrationService::sendToACU(const QByteArray& data)
{
    auto pack = AGCCurveCalibrationServiceImpl::packServerSelf("sendToACU", "sendToACUACK", data);
    silenceSend(pack);
}

void AGCCurveCalibrationService::selectAGCDataACK(const ProtocolPack& pack) { emit sg_selectAGCDataACK(pack.data); }
void AGCCurveCalibrationService::saveAGCDataACK(const ProtocolPack& pack) { emit sg_saveAGCDataACK(pack.data); }
void AGCCurveCalibrationService::startAGCDataACK(const ProtocolPack& pack) { emit sg_startAGCDataACK(pack.data); }
void AGCCurveCalibrationService::endAGCDataACK(const ProtocolPack& pack) { emit sg_endAGCDataACK(pack.data); }
void AGCCurveCalibrationService::deleteAGCDataACK(const ProtocolPack& pack) { emit sg_deleteAGCDataACK(pack.data); }
void AGCCurveCalibrationService::finishingItemACK(const ProtocolPack& pack) { emit sg_finishingItemACK(pack.data); }
void AGCCurveCalibrationService::sendToMACBACK(const ProtocolPack& pack) { emit sg_sendToMACBACK(pack.data); }
void AGCCurveCalibrationService::sendToACUACK(const ProtocolPack& pack) { emit sg_sendToACUACK(pack.data); }
