#ifndef DEVICEXMLLOADSERVICE_H
#define DEVICEXMLLOADSERVICE_H

#include "INetMsg.h"
#include "ProtocolXmlTypeDefine.h"

namespace cppmicroservices
{
    class BundleContext;
}
class DeviceXmlLoadServiceImpl;
class DeviceXmlLoadService : public INetMsg
{
public:
    DeviceXmlLoadService(cppmicroservices::BundleContext context);
    ~DeviceXmlLoadService();

    void loadDeviceXMLConfig();  //加载单元 命令 系统配置文件
    QMap<int, System> getSystemData() const;
    Unit getUnitData(int sysID, int deviceID, int extentID, int modeID, int unitID) const;
    Extension getExtension(int sysID, int deviceID, int extentID) const;
    CmdAttribute getCmdData(int sysID, int deviceID, int extentID, int modeID, int cmdID) const;

private:
    DeviceXmlLoadServiceImpl* m_impl;
};
#endif  // DEVICEXMLLOADSERVICE_H
