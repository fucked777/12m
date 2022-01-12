#include "AddRemovePowerService.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessMessageSerialize.h"
#include "Utility.h"
#include <QMessageBox>

class AddRemovePowerServiceImpl
{
    DIS_COPY_MOVE(AddRemovePowerServiceImpl)
public:
    inline AddRemovePowerServiceImpl() {}
    inline ~AddRemovePowerServiceImpl() {}
    cppmicroservices::BundleContext context;
};
AddRemovePowerService::AddRemovePowerService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new AddRemovePowerServiceImpl)
{
    m_impl->context = context;
}

AddRemovePowerService::~AddRemovePowerService() { delete m_impl; }

void AddRemovePowerService::slotsUnitDeviceJson(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientUnitSettingMessage";
    pack.data = json.toUtf8();

    this->send(pack);
}

void AddRemovePowerService::slotsCmdDeviceJson(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientCmdSettingMessage";
    pack.data = json.toUtf8();

    this->send(pack);
}
