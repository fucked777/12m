#ifndef ADDREMOVEPOWERSERVICE_H
#define ADDREMOVEPOWERSERVICE_H

#include "DeviceProcessMessageDefine.h"
#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

class AddRemovePowerServiceImpl;
class AddRemovePowerService : public INetMsg
{
    Q_OBJECT
public:
    AddRemovePowerService(cppmicroservices::BundleContext context);
    ~AddRemovePowerService();

public:
    /*发送单元设置命令*/
    void slotsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void slotsCmdDeviceJson(QString json);

private:
    AddRemovePowerServiceImpl* m_impl;
};
#endif  // ADDREMOVEPOWERSERVICE_H
