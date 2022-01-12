#include "CaptureTrackIng.h"
#include "TaskGlobalInfo.h"
#include <QDebug>
#include <QThread>

// int CaptureTrackIng::typeId = qRegisterMetaType<CaptureTrackIng>();
CaptureTrackIng::CaptureTrackIng(QObject* parent)
    : BaseEvent(parent)
{
}

CaptureTrackIng::~CaptureTrackIng() {}

void CaptureTrackIng::doSomething()
{
    DeviceWorkTask deviceWorkTask = taskMachine->deviceWorkTask();
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    auto m_ckDeviceMap = taskTimeList.m_ckDeviceMap;
    auto m_ckQdDeviceMap = taskTimeList.m_ckQdDeviceMap;
    auto m_gzDeviceMap = taskTimeList.m_gzDeviceMap;
    auto m_gzQdDeviceMap = taskTimeList.m_gzQdDeviceMap;
    auto m_dsDeviceMap = taskTimeList.m_dsDeviceMap;
    auto m_gsDeviceMap = taskTimeList.m_gsDeviceMap;

    auto currentDateTime = GlobalData::currentDateTime();
    while (taskTimeList.m_track_start_time > currentDateTime)
    {
        QThread::msleep(800);
        currentDateTime = GlobalData::currentDateTime();
    };

    notifyInfoLog("开始自跟踪");

    //    // 监控收到校相结果后向ACU下发任务开始命令
    //    auto taskIdent = TaskGlobalInfoSingleton::getInstance().getInstance().taskIdent(uuid);
    //    QMap<QString, QVariant> acuTaskStartParamMap;
    //    acuTaskStartParamMap["TaskIdent"] = taskIdent;  // 任务标识
    //    cmd.getTaskPlanCmd(array, QString("Step_AC_14_1"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID, m_runningCmdID,
    //                       acuTaskStartParamMap);
    //    waitExecSuccess(array);

    //    // 天线转自跟踪(当测控基带为标准TTC或者S扩频时天线设置为S频段，Ka扩频时设置为Ka频段)
    //    if (currentCkWorkMode.STTC || currentCkWorkMode.SKuo2)
    //    {
    //        // S频段天线自动跟踪
    //        cmd.getTaskPlanCmd(array, QString("Step_AC_7"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID, m_runningCmdID);
    //        waitExecSuccess(array);
    //    }
    //    else if (currentCkWorkMode.KaKuo2)
    //    {
    //        // Ka频段天线自动跟踪
    //        cmd.getTaskPlanCmd(array, QString("Step_AC_6"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID, m_runningCmdID);
    //        waitExecSuccess(array);
    //    }

    // 获取基带载波状态和AGC值  暂定。
    // ？？？？？

    QThread::sleep(1);
    notify("GoUp");
}
