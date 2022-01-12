#include "SystemBlockDiagramService.h"
#include "CppMicroServicesUtility.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "Utility.h"
#include <QMessageBox>

static constexpr auto serverSelfModule = "ServerDeviceProcessService";
class SystemBlockDiagramServiceImpl
{
    DIS_COPY_MOVE(SystemBlockDiagramServiceImpl)
public:
    inline SystemBlockDiagramServiceImpl() {}
    inline ~SystemBlockDiagramServiceImpl() {}
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
SystemBlockDiagramService::SystemBlockDiagramService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new SystemBlockDiagramServiceImpl)
{
    m_impl->context = context;

    registerSubscribe("AcceptDeviceCMDResponceMessage", &SystemBlockDiagramService::AcceptDeviceCMDResponceMessage, this);
}

SystemBlockDiagramService::~SystemBlockDiagramService() { delete m_impl; }

void SystemBlockDiagramService::sendUnitDeviceJson(const QString& json)
{
    auto pack = SystemBlockDiagramServiceImpl::packServerSelf("AcceptClientUnitSettingMessage", "", json.toUtf8());
    silenceSend(pack);
}

void SystemBlockDiagramService::sendProcessDeviceJson(const QString& json)
{
    auto pack = SystemBlockDiagramServiceImpl::packServerSelf("ServerDeviceProcessService", "", json.toUtf8());
    silenceSend(pack);
}

void SystemBlockDiagramService::AcceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    ControlCmdResponse cmdRespnce;
    auto tempData = pack.data;
    tempData >> cmdRespnce;

    auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdRespnce.responseResult);
    QMessageBox::information(nullptr, QString("提示"), QString(result), QString("确定"));
}
