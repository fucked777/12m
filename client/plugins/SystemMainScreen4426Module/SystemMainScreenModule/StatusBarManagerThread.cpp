#include "StatusBarManagerThread.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"
#include "UserManagerMessageDefine.h"
#include <QDebug>
#include "RedisHelper.h"
StatusBarManagerThread::StatusBarManagerThread()
{
    qRegisterMetaType<StatusBarData>("const StatusBarData&");
    mCurProject = ExtendedConfig::curProjectID();
}

void StatusBarManagerThread::stopRunning() { m_isRunning = false; }

void StatusBarManagerThread::run()
{
    auto systemMap = GlobalData::getSystemMap();
    while (m_isRunning)
    {
        m_statusBarData.deviceIDOffName.clear();
        m_statusBarData.deviceIDSubName.clear();

        for (auto sysID : systemMap.keys())
        {
            const auto system = systemMap.value(sysID);
            for (auto devID : system.deviceMap.keys())
            {
                auto device = system.deviceMap.value(devID);
                for (auto extension : device.extensionMap)
                {
                    DeviceID deviceID(sysID, devID, extension.ID);
                    if (!GlobalData::getDeviceOnlineByObject(sysID, devID, extension.ID))
                    {
                        m_statusBarData.deviceIDOffName[deviceID] = extension.desc;
                    }
                    else if (!GlobalData::getDeviceSelfControl(sysID, devID, extension.ID))
                    {
                        m_statusBarData.deviceIDSubName[deviceID] = extension.desc;
                    }
                }
            }
        }

        DeviceID deviceCKA1(0x4001);
        DeviceID deviceCKA2(0x4002);
        DeviceID deviceCKB1(0x4003);
        DeviceID deviceCKB2(0x4004);

        //下面的处理是针对单模式的处理
        if (!m_statusBarData.deviceIDOffName.contains(deviceCKA1) && m_statusBarData.deviceIDOffName.contains(deviceCKA2))
        {
            m_statusBarData.deviceIDOffName.remove(deviceCKA2);
        }

        if (m_statusBarData.deviceIDOffName.contains(deviceCKA1) && !m_statusBarData.deviceIDOffName.contains(deviceCKA2))
        {
            m_statusBarData.deviceIDOffName.remove(deviceCKA1);
        }

        if (!m_statusBarData.deviceIDOffName.contains(deviceCKB1) && m_statusBarData.deviceIDOffName.contains(deviceCKB2))
        {
            m_statusBarData.deviceIDOffName.remove(deviceCKB2);
        }

        if (m_statusBarData.deviceIDOffName.contains(deviceCKB1) && !m_statusBarData.deviceIDOffName.contains(deviceCKB2))
        {
            m_statusBarData.deviceIDOffName.remove(deviceCKB1);
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

        //存储转发送数异常语音播报
        cczfSendDataSound();

        // 服务器主备状态
        m_statusBarData.serverMasterText = "A主B备";

        emit signalRefreshUI(m_statusBarData);

        QThread::sleep(1);
    }
}

void StatusBarManagerThread::cczfSendDataSound()
{
    int cczfAUnit6 = GlobalData::getReportParamData(0x8101, 6, "WorkStatus").toInt();//存转A转发单元1
    int cczfAUnit7 = GlobalData::getReportParamData(0x8101, 7, "WorkStatus").toInt();//存转A转发单元2
    int cczfAUnit8 = GlobalData::getReportParamData(0x8101, 8, "WorkStatus").toInt();//存转A转发单元3
    int cczfAUnit9 = GlobalData::getReportParamData(0x8101, 9, "WorkStatus").toInt();//存转A转发单元4

    int cczfBUnit6 = GlobalData::getReportParamData(0x8102, 6, "WorkStatus").toInt();//存转B转发单元1
    int cczfBUnit7 = GlobalData::getReportParamData(0x8102, 7, "WorkStatus").toInt();//存转B转发单元2
    int cczfBUnit8 = GlobalData::getReportParamData(0x8102, 8, "WorkStatus").toInt();//存转B转发单元3
    int cczfBUnit9 = GlobalData::getReportParamData(0x8102, 9, "WorkStatus").toInt();//存转B转发单元4
    QString cczfA = "存储转发A";
    QString cczfB = "存储转发B";
    auto dealWithCCZFSendData = [](const int workStatus,int zfUnit, QString cczfName) {
        auto cczfZFUnitName = QString("转发单元%1").arg(QString::number(zfUnit));
        if (workStatus == 5)
        {
            auto str = cczfName + cczfZFUnitName + "转发错误";
            RedisHelper::getInstance().publistMusic(str);
        }
        else if (workStatus == 6)
        {
            auto str = cczfName + cczfZFUnitName + "时延过大";
            RedisHelper::getInstance().publistMusic(str);
        }
    };

    dealWithCCZFSendData(cczfAUnit6,1,cczfA);
    dealWithCCZFSendData(cczfAUnit7,2,cczfA);
    dealWithCCZFSendData(cczfAUnit8,3,cczfA);
    dealWithCCZFSendData(cczfAUnit9,4,cczfA);

    dealWithCCZFSendData(cczfBUnit6,1,cczfB);
    dealWithCCZFSendData(cczfBUnit7,2,cczfB);
    dealWithCCZFSendData(cczfBUnit8,3,cczfB);
    dealWithCCZFSendData(cczfBUnit9,4,cczfB);
}
