#include "StatusBarManagerThread.h"
#include "GlobalData.h"
#include "ExtendedConfig.h"
#include "ProtocolXmlTypeDefine.h"
#include "UserManagerMessageDefine.h"
#include <QDebug>

StatusBarManagerThread::StatusBarManagerThread()
{
    qRegisterMetaType<StatusBarData>("const StatusBarData&");
    mCurProject = ExtendedConfig::curProjectID();
}

void StatusBarManagerThread::stopRunning() { m_isRunning = false; }

void StatusBarManagerThread::run()
{
    while (m_isRunning)
    {
        m_statusBarData.offlineDeviceList.clear();
        m_statusBarData.subCtrlDeviceList.clear();

        auto systemMap = GlobalData::getSystemMap();
        for (auto sysID : systemMap.keys())
        {
            const auto system = systemMap.value(sysID);
            for (auto devID : system.deviceMap.keys())
            {
                auto device = system.deviceMap.value(devID);
                for (auto extension : device.extensionMap)
                {
                    if (!GlobalData::getDeviceOnline(sysID, devID, extension.ID))
                    {
                        m_statusBarData.offlineDeviceList.append(extension.desc);
                    }
                    else if (!GlobalData::getDeviceSelfControl(sysID, devID, extension.ID))
                    {
                        m_statusBarData.subCtrlDeviceList.append(extension.desc);
                    }
                }
            }
        }

        // 只有4426
        if (mCurProject == "4426")
        {
            // 获取S校零变频器加去电状态
            auto sZeroCalibration = GlobalData::getReportParamData(0x6901, 1, "AddRemElecCon");
            if (sZeroCalibration.toInt() == 2)
            {
                m_statusBarData.SZeroCalibrationOn = true;
            }
            else
            {
                m_statusBarData.SZeroCalibrationOn = false;
            }
            // 获取Ka校零变频器加去电状态
            auto kaZeroCalibration = GlobalData::getReportParamData(0x6902, 1, "AddRemElecCon");
            if (kaZeroCalibration.toInt() == 2)
            {
                m_statusBarData.KaZeroCalibrationOn = true;
            }
            else
            {
                m_statusBarData.KaZeroCalibrationOn = false;
            }
        }

        // 服务器主备状态
        m_statusBarData.serverMasterText = "A主B备";

        emit signalRefreshUI(m_statusBarData);

        QThread::sleep(1);
    }
}
