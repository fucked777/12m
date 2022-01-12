#ifndef DEVICEPROCESSSERVICE_H
#define DEVICEPROCESSSERVICE_H

#include "DeviceProcessMessageDefine.h"
#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

class DeviceProcessServiceImpl;
class DeviceProcessService : public INetMsg
{
    Q_OBJECT
public:
    DeviceProcessService(cppmicroservices::BundleContext context);
    ~DeviceProcessService();

signals:

    /*单元参数设置命令*/
    void signalsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

    /*分机状态上报的数据*/
    void signalsStatusReportJson(StatusReportingRequest statusReport);

    /*分机过程控制命令响应 */
    void signalsCMDResponceJson(ControlCmdResponse cmdRespnce);

    /*分机过程控制命令结果上报 */
    void signalsCMDResultJson(CmdResult cmdRespnce);

public:
    /*发送单元设置命令*/
    void slotsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void slotsCmdDeviceJson(QString json);

    /*发送过参数宏命令*/
    void slotsParaMacroDeviceJson(QString json);

public:
    /*发送数据给设备 QByteArray*/
    //    void SendDeviceMessage(const ProtocolPack& pack);
    /*接收设备数据 QByteArray*/
    //    void AcceptDeviceMessage(const ProtocolPack& pack);
    /*接收到设备异常的数据 QByteArray*/
    void AcceptErrorDeviceMessage(const ProtocolPack& pack);
    /*接收设备状态上报数据 QByteArray*/
    void AcceptDeviceStatusReportingMessage(const ProtocolPack& pack);
    /*接收设备过程控制命令响应 QByteArray*/
    void AcceptDeviceCMDResponceMessage(const ProtocolPack& pack);
    /*接收设备过程控制命令结果上报 QByteArray*/
    void AcceptDeviceCMDResultMessage(const ProtocolPack& pack);

private:
    DeviceProcessServiceImpl* m_impl;
};
#endif  // DEVICEPROCESSSERVICE_H
