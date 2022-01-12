#include "DeviceDataLoadThread.h"
#include <QMutexLocker>
#include <QMap>
#include "PlatformObjectTools.h"
#include "ProtocolXmlTypeDefine.h"
DeviceDataLoadThread::DeviceDataLoadThread() {}

void DeviceDataLoadThread::stopRunning() { m_isRunning = false; }


void DeviceDataLoadThread::run()
{
    //定义所有需要缓存的数据信息，在主界面刷新的时候不用重新去读取数据，这里通过一个线程直接读取数据。by wjy
    QMap<DeviceID,ExtensionStatusReportMessage> m_extensionStatusReportMessageMap;
    QMap<DeviceID,bool> m_deviceOnlineMap;
    QMap<DeviceID,int> m_deviceModeMap;

    auto systemMap = GlobalData::getSystemMap();
    QList<DeviceID> mAllDeviceID;
    for (auto sysID : systemMap.keys())
    {
        const auto system = systemMap.value(sysID);
        for (auto devID : system.deviceMap.keys())
        {
            auto device = system.deviceMap.value(devID);
            for (auto extensionID : device.extensionMap.keys())
            {
                mAllDeviceID.append(DeviceID(sysID,devID,extensionID));
            }
        }
    }
    while (m_isRunning)
    {

        for(auto deviceID : mAllDeviceID){
            auto extensionStatusReportMessage = GlobalData::getExtenStatusReportData(deviceID);
            bool onlineStatus = GlobalData::getDeviceOnline(deviceID);
            int deviceMode = GlobalData::getOnlineDeviceModeId(deviceID);
            m_extensionStatusReportMessageMap[deviceID] =  extensionStatusReportMessage;
            m_deviceOnlineMap[deviceID] =  onlineStatus;
            m_deviceModeMap[deviceID] =  deviceMode;
        }

       auto platformObjectTools = PlatformObjectTools::instance();
       platformObjectTools->updataObj<QMap<DeviceID,ExtensionStatusReportMessage>>("ALLExtensionStatusReportMap", m_extensionStatusReportMessageMap);
       platformObjectTools->updataObj<QMap<DeviceID,bool>>("ALLDeviceOnlineStatusMap", m_deviceOnlineMap);
       platformObjectTools->updataObj<QMap<DeviceID,int>>("ALLDeviceModeStatusMap", m_deviceModeMap);

        QThread::sleep(1);
    }
}
