#ifndef DEVICEPROCESSSERVICE_H
#define DEVICEPROCESSSERVICE_H

#include "INetMsg.h"
#include <QScopedPointer>

struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

struct PackMessage;
struct UnpackMessage;
struct MessageAddress;
struct CmdResult;
enum class DevMsgType : quint32;
class DeviceProcessServiceImpl;
class DeviceProcessService : public INetMsg
{
    Q_OBJECT
public:
    DeviceProcessService(const QString& threadName, cppmicroservices::BundleContext context);
    ~DeviceProcessService();

private:
    /*发送数据给设备*/
    void sendByteArrayToDevice(const QByteArray& array, const QString& cmdInfo);
    /*接收设备数据 QByteArray*/
    void acceptDeviceMessage(const ProtocolPack& pack);
    /*接收到设备异常的数据 QByteArray*/
    void acceptErrorDeviceMessage(const ProtocolPack& pack);

    /*接收客户端单元参数设置命令*/
    void acceptClientUnitSettingMessage(const ProtocolPack& pack);

    /*接收客户端命令参数设置命令*/
    void acceptClientCmdSettingMessage(const ProtocolPack& pack);

    /*接收客户端参数宏设置命令*/
    void acceptClientParamMacroSettingMessage(const ProtocolPack& pack);

private:
    QScopedPointer<DeviceProcessServiceImpl> m_impl;
};
#endif  // DEVICEPROCESSSERVICE_H
