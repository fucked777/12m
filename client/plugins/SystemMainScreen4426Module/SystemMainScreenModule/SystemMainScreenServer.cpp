#include "SystemMainScreenServer.h"

#include "CppMicroServicesUtility.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "ExtendedConfig.h"
#include <QMessageBox>

class SystemMainScreenServerImpl
{
    DIS_COPY_MOVE(SystemMainScreenServerImpl)
public:
    inline SystemMainScreenServerImpl() {}
    inline ~SystemMainScreenServerImpl() {}
    cppmicroservices::BundleContext context;
};

SystemMainScreenServer::SystemMainScreenServer(cppmicroservices::BundleContext context)
    : INetMsg()
    , mImpl(new SystemMainScreenServerImpl)
{
    mImpl->context = context;

    registerSubscribe("AcceptDeviceCMDResponceMessage", &SystemMainScreenServer::AcceptDeviceCMDResponceMessage, this);
    registerSubscribe("AcceptDeviceCMDResultMessage", &SystemMainScreenServer::AcceptDeviceCMDResultMessage, this);

    d_CurProjectId = ExtendedConfig::curProjectID();
}

SystemMainScreenServer::~SystemMainScreenServer() { delete mImpl; }

void SystemMainScreenServer::slotSendProcessCmd(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientCmdSettingMessage";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SystemMainScreenServer::slotSendUnitParam(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientUnitSettingMessage";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SystemMainScreenServer::slotManualFunction(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerAutoTaskService";
    pack.module = false;
    pack.operation = "manualOperation";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SystemMainScreenServer::AcceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    auto tempData = pack.data;

    emit signalsCMDResponceJson(tempData);
}

void SystemMainScreenServer::AcceptDeviceCMDResultMessage(const ProtocolPack& pack)
{
    auto tempData = pack.data;

    emit signalsDeviceCMDResult(tempData);
}
