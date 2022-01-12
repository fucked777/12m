#ifndef SYSTEMBLOCKDIAGRAMSERVICE_H
#define SYSTEMBLOCKDIAGRAMSERVICE_H

#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

class SystemBlockDiagramServiceImpl;
class SystemBlockDiagramService : public INetMsg
{
    Q_OBJECT
public:
    SystemBlockDiagramService(cppmicroservices::BundleContext context);
    ~SystemBlockDiagramService();

public:
    /*发送单元设置命令*/
    void sendUnitDeviceJson(const QString& json);

    /*发送过程控制命令*/
    void sendProcessDeviceJson(const QString& json);

public slots:
    /*接收设备过程控制命令响应 QByteArray*/
    void AcceptDeviceCMDResponceMessage(const ProtocolPack& pack);

private:
    SystemBlockDiagramServiceImpl* m_impl;
};
#endif  // SYSTEMBLOCKDIAGRAMSERVICE_H
