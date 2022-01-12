#include "DataTransmissionCenterService.h"
#include "CppMicroServicesUtility.h"

static constexpr auto serverSelfModule = "DataTransmissionCenter";
class DataTransmissionCenterServiceImpl
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
DataTransmissionCenterService::DataTransmissionCenterService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DataTransmissionCenterServiceImpl)
{
    m_impl->context = context;
    registerSubscribe("addItemACK", &DataTransmissionCenterService::addItemACK, this);
    registerSubscribe("deleteItemACK", &DataTransmissionCenterService::deleteItemACK, this);
    registerSubscribe("modifyItemACK", &DataTransmissionCenterService::modifyItemACK, this);
}

DataTransmissionCenterService::~DataTransmissionCenterService() { delete m_impl; }

void DataTransmissionCenterService::addItem(const QByteArray& data)
{
    auto pack = DataTransmissionCenterServiceImpl::packServerSelf("addItem", "addItemACK", data);
    silenceSend(pack);
}
void DataTransmissionCenterService::deleteItem(const QByteArray& data)
{
    auto pack = DataTransmissionCenterServiceImpl::packServerSelf("deleteItem", "deleteItemACK", data);
    silenceSend(pack);
}
void DataTransmissionCenterService::modifyItem(const QByteArray& data)
{
    auto pack = DataTransmissionCenterServiceImpl::packServerSelf("modifyItem", "modifyItemACK", data);
    silenceSend(pack);
}

void DataTransmissionCenterService::addItemACK(const ProtocolPack& pack) { emit sg_addACK(pack.data); }
void DataTransmissionCenterService::deleteItemACK(const ProtocolPack& pack) { emit sg_deleteACK(pack.data); }
void DataTransmissionCenterService::modifyItemACK(const ProtocolPack& pack) { emit sg_modifyACK(pack.data); }
