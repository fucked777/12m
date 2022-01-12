#include "DeviceProcessService.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessMessageSerialize.h"
#include "Utility.h"
#include <QMessageBox>

class DeviceProcessServiceImpl
{
    DIS_COPY_MOVE(DeviceProcessServiceImpl)
public:
    inline DeviceProcessServiceImpl() {}
    inline ~DeviceProcessServiceImpl() {}
    cppmicroservices::BundleContext context;
};
DeviceProcessService::DeviceProcessService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new DeviceProcessServiceImpl)
{
    m_impl->context = context;
    qRegisterMetaType<StatusReportingRequest>("StatusReportingRequest");
    qRegisterMetaType<ControlCmdResponse>("ControlCmdResponse");

    registerSubscribe("AcceptDeviceStatusReportingMessage", &DeviceProcessService::AcceptDeviceStatusReportingMessage, this, false);
    registerSubscribe("AcceptDeviceCMDResponceMessage", &DeviceProcessService::AcceptDeviceCMDResponceMessage, this);
    registerSubscribe("AcceptDeviceCMDResultMessage", &DeviceProcessService::AcceptDeviceCMDResultMessage, this);

    registerSubscribe("AcceptErrorDeviceMessage", &DeviceProcessService::AcceptErrorDeviceMessage, this);
}

DeviceProcessService::~DeviceProcessService() { delete m_impl; }

void DeviceProcessService::slotsUnitDeviceJson(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientUnitSettingMessage";
    pack.data = json.toUtf8();

    this->send(pack);
}

void DeviceProcessService::slotsCmdDeviceJson(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientCmdSettingMessage";
    pack.data = json.toUtf8();

    this->send(pack);
}

void DeviceProcessService::slotsParaMacroDeviceJson(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ServerDeviceProcessService";
    pack.module = false;
    pack.operation = "AcceptClientParamMacroSettingMessage";
    pack.data = json.toUtf8();
    this->send(pack);
}

void DeviceProcessService::AcceptDeviceStatusReportingMessage(const ProtocolPack& pack)
{
    StatusReportingRequest statusReportRequest;
    JsonReader reader(pack.data);
    reader& statusReportRequest;

    emit signalsStatusReportJson(statusReportRequest);
}

void DeviceProcessService::AcceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    ControlCmdResponse cmdRespnce;
    JsonReader reader(pack.data);
    reader& cmdRespnce;

    emit signalsCMDResponceJson(cmdRespnce);
    qDebug() << "AcceptDeviceCMDResponceMessage";
}

void DeviceProcessService::AcceptDeviceCMDResultMessage(const ProtocolPack& pack)
{
    CmdResult cmdResult;
    JsonReader reader(pack.data);
    reader& cmdResult;

    emit signalsCMDResultJson(cmdResult);
}

void DeviceProcessService::AcceptErrorDeviceMessage(const ProtocolPack& pack) {}
