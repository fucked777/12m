#include "DeviceXmlLoadService.h"
#include "GlobalData.h"
#include "Protocol/ProtocolXMLCmdReader.h"
#include "Protocol/ProtocolXMLUnitReader.h"
#include "SystemConfigXMLReader.h"
#include "Utility.h"
#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"

#include <QApplication>
#include <QDebug>
#include <QDir>

class DeviceXmlLoadServiceImpl
{
public:
    cppmicroservices::BundleContext context;
};

DeviceXmlLoadService::DeviceXmlLoadService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new DeviceXmlLoadServiceImpl)
{
    m_impl->context = context;
}

DeviceXmlLoadService::~DeviceXmlLoadService() { delete m_impl; }

void DeviceXmlLoadService::loadDeviceXMLConfig()
{
    ProtocolXMLUnitReader protocolXMLUnitReader;
    ProtocolXMLCmdReader protocolXMLCmdReader;
    SystemConfigXMLReader systemConfigXMLReader;

    QMap<int, System> systemMap;
    /*@brief 增加项目配置文件 add code start*/
    auto deviceXmlDir = PlatformConfigTools::configShareDir();

    //加载单元数据
    QString filePath = QString("%1/QueryAndParaSet/").arg(deviceXmlDir);
    auto unitXMLData = protocolXMLUnitReader.loadUnitProtocolXml(filePath);
    //加载过程控制命令数据
    filePath = QString("%1/Command/").arg(deviceXmlDir);
    auto cmdXMLData = protocolXMLCmdReader.loadCmdProtocolXml(filePath);
    //加载系统配置数据
    filePath = QString("%1/SystemConfig.xml").arg(deviceXmlDir);
    systemConfigXMLReader.loadSystemConfigXML(filePath, systemMap, unitXMLData, cmdXMLData);

    GlobalData::setSystemMap(systemMap);
}

QMap<int, System> DeviceXmlLoadService::getSystemData() const { return GlobalData::getSystemMap(); }

Unit DeviceXmlLoadService::getUnitData(int sysID, int deviceID, int extentID, int modeID, int unitID) const
{
    auto systemData = GlobalData::getSystemMap();
    return systemData[sysID].deviceMap[deviceID].extensionMap[extentID].modeCtrlMap[modeID].unitMap[unitID];
}

Extension DeviceXmlLoadService::getExtension(int sysID, int deviceID, int extentID) const
{
    auto systemData = GlobalData::getSystemMap();
    return systemData[sysID].deviceMap[deviceID].extensionMap[extentID];
}

CmdAttribute DeviceXmlLoadService::getCmdData(int sysID, int deviceID, int extentID, int modeID, int cmdID) const
{
    auto systemData = GlobalData::getSystemMap();
    return systemData[sysID].deviceMap[deviceID].extensionMap[extentID].modeCtrlMap[modeID].cmdMap[cmdID];
}
