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
    uint secs = d_lastTime.secsTo(QTime::currentTime());
    if (secs < 3)
    {
        return;
    }
    d_lastTime = QTime::currentTime();

    ControlCmdResponse cmdRespnce;
    auto tempData = pack.data;
    tempData >> cmdRespnce;

    auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdRespnce.responseResult);
    if (d_CurProjectId == "4424")
    {
        int deviceId = -1;
        cmdRespnce.deviceID >> deviceId;

        if (deviceId == 0x4001 || deviceId == 0x4002)
        {
            DevMsgType type = cmdRespnce.cmdType;
            int cmdId = cmdRespnce.cmdId;

            if ((type == DevMsgType::ProcessControlCmd && cmdId == 48) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 4))
            {
                return;
            }
            if ((type == DevMsgType::ProcessControlCmd && cmdId == 24) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 3))
            {
                return;
            }
            if ((type == DevMsgType::ProcessControlCmd && cmdId == 7) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 3))
            {
                return;
            }
            //报上来的modeId是0
            //            switch (modeId)
            //            {
            //            case 0x01:
            //            {
            //                if ((type == DevMsgType::ProcessControlCmd && cmdId == 48) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 4))
            //                {
            //                    return;
            //                }
            //            }
            //            case 0x04:
            //            case 0x10:
            //            case 0x11:
            //            case 0x12:
            //            case 0x13:
            //            case 0x15:
            //            case 0x17:
            //            case 0x19:
            //            {
            //                if ((type == DevMsgType::ProcessControlCmd && cmdId == 24) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 3))
            //                {
            //                    return;
            //                }
            //            }
            //            case 0x21:
            //            {
            //                if ((type == DevMsgType::ProcessControlCmd && cmdId == 7) || (type == DevMsgType::UnitParameterSetCmd && cmdId == 3))
            //                {
            //                    return;
            //                }
            //            }
            //            default: break;
            //            }
        }
    }
    QMessageBox::information(nullptr, QString("提示"), QString(result), QString("确定"));
}

void SystemMainScreenServer::AcceptDeviceCMDResultMessage(const ProtocolPack& pack)
{
    CmdResult cmdResult;
    auto tempData = pack.data;
    tempData >> cmdResult;

    emit signalsDeviceCMDResult(tempData);
}
