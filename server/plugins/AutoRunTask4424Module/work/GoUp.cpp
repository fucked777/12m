#include "GoUp.h"
#include "TaskGlobalInfo.h"
#include <QDebug>
#include <QThread>
// int GoUp::typeId = qRegisterMetaType<GoUp>();
GoUp::GoUp(QObject* parent)
    : BaseEvent(parent)
{
}

void GoUp::doSomething()
{
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    QMap<int, DeviceID> m_ckDeviceMap = taskTimeList.m_ckDeviceMap;      //测控模式下的测控基带信息
    QMap<int, DeviceID> m_ckQdDeviceMap = taskTimeList.m_ckQdDeviceMap;  //测控前端信息
    QMap<int, DeviceID> m_gzDeviceMap = taskTimeList.m_gzDeviceMap;      //跟踪基带信息
    QMap<int, DeviceID> m_gzQdDeviceMap = taskTimeList.m_gzQdDeviceMap;  //跟踪前端信息
    QMap<int, DeviceID> m_dsDeviceMap = taskTimeList.m_dsDeviceMap;      //低速基带信息
    QMap<int, DeviceID> m_gsDeviceMap = taskTimeList.m_gsDeviceMap;      //高速基带信息
    QMap<int, DeviceID> m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;    //功放的信息

    PackCommand packCommand;
    /***
     * T4 开上行载波时间
     * 1.系统监控根据设备工作计划中的开上行信号时间控制功放信号上天线
     * 2.系统监控根据设备工作计划中遥控开始时间发送遥控信号
     ***/

    QDateTime currentDateTime;
    int number;

    /*** 1.系统监控根据设备工作计划中的开上行信号时间控制功放信号上天线 ***/
    auto onUpLinkSignalTimeString = taskTimeList.m_on_uplink_signal_time;
    QDateTime onUpLinkSignalTime;
    // 若设备工作计划中该项为全0，则不发上行信号
    if (onUpLinkSignalTimeString == TimeZero)
    {
        notifyInfoLog("开上行信号时间为全为0，不发上行信号");
        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
    }
    else
    {
        // 若为全F，则在T0时刻发送上行信号（立即发）
        if (onUpLinkSignalTimeString == TimeF)
        {
            onUpLinkSignalTime = GlobalData::currentDateTime();
        }
        else
        {
            onUpLinkSignalTime = QDateTime::fromString(onUpLinkSignalTimeString, ORIGINALFORMAT);
        }

        currentDateTime = GlobalData::currentDateTime();
        number = -1;
        while (onUpLinkSignalTime >= currentDateTime)
        {
            // 如果退出就直接返回
            if (m_isExit)
                return;

            number++;

            QThread::msleep(100);
            currentDateTime = GlobalData::currentDateTime();
        }
        notifyInfoLog("发上行流程开始执行");
        // TODO 记录发上行时间 和动作

        //功放发上行  S Ka测控 Ka低速

        if (taskTimeList.m_dataMap.contains(STTC))
        {
            //基带上行载波输出
            m_singleCommandHelper.packSingleCommand("Step_STTC_Carrier_Output", packCommand, m_ckDeviceMap[STTC]);
            waitExecSuccess(packCommand);
        }

        if (taskTimeList.m_dataMap.contains(Skuo2))
        {
            //基带上行载波输出
            m_singleCommandHelper.packSingleCommand("Step_CK_S_5_OUT", packCommand, m_ckDeviceMap[Skuo2]);
            waitExecSuccess(packCommand);
        }

        if (taskTimeList.m_dataMap.contains(KaKuo2))
        {
            //设置测控功放输出功率
            m_singleCommandHelper.packSingleCommand("Step_HPA_4_1_START", packCommand, m_ckDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);

            //设置ka测控基带载波输出
            m_singleCommandHelper.packSingleCommand("Step_CK_KA_5_OUT", packCommand, m_ckDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);
        }

        if (taskTimeList.m_dataMap.contains(KaDS))
        {
            //设置数传功放输出功率
            m_singleCommandHelper.packSingleCommand("Step_SCHPA_4_1_START", packCommand, m_hpaDeviceMap[KaDS]);
            waitExecSuccess(packCommand);
        }

        notifyInfoLog("发上行流程完成");
    }

    /*** 2.系统监控根据设备工作计划中遥控开始时间发送遥控信号 ***/
    auto remoteControlStartTimeString = taskTimeList.m_remote_start_time;
    QDateTime remoteControlStartTime;
    // 若设备工作计划中该项为全0，则不发上行信号
    if (remoteControlStartTimeString == TimeZero)
    {
        notifyInfoLog("遥控开始时间为全为0，不发遥控信号");
    }
    else
    {
        // 若为全F，则在T0时刻发送上行信号(立即发)
        if (remoteControlStartTimeString == TimeF)
        {
            remoteControlStartTime = GlobalData::currentDateTime();
        }
        else
        {
            remoteControlStartTime = QDateTime::fromString(remoteControlStartTimeString, ORIGINALFORMAT);
        }

        currentDateTime = GlobalData::currentDateTime();
        number = -1;
        while (remoteControlStartTime > currentDateTime)
        {
            // 如果退出就直接返回
            if (m_isExit)
                return;

            number++;
            if (number % 5 == 0)
            {
                QString info = QString("发送遥控信号还有%1秒开始执行").arg(currentDateTime.secsTo(remoteControlStartTime));
                notifyInfoLog(info);
            }
            QThread::sleep(1);
            currentDateTime = GlobalData::currentDateTime();
        }
        notifyInfoLog("发送遥控信号开始执行");

        // 测控基带加调遥控信号
        if (taskTimeList.m_dataMap.contains(STTC))
        {
            DeviceID deviceId = m_ckDeviceMap[STTC];
            m_singleCommandHelper.packSingleCommand("Step_STTC_RemoteControl_ON", packCommand, deviceId);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(Skuo2))
        {
            DeviceID deviceId = m_ckDeviceMap[Skuo2];
            m_singleCommandHelper.packSingleCommand("Step_CK_S_6_2", packCommand, deviceId);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(KaKuo2))
        {
            DeviceID deviceId = m_ckDeviceMap[KaKuo2];
            m_singleCommandHelper.packSingleCommand("Step_CK_KA_6_2", packCommand, deviceId);
            waitExecSuccess(packCommand);
        }

        notifyInfoLog("发送遥控信号完成");
    }

    /***
     * T5 关上行载波时间
     * 系统监控按照设备工作计划控制测控基带停发上行、去调遥控信号。并控制功率下天线。
     ***/
    auto turnOffLineSignalTimeString = taskTimeList.m_turn_off_line_signal_time;
    QDateTime turnOffLineSignalTime;
    // 若设备工作计划中该项为全0
    if (turnOffLineSignalTimeString == TimeZero)
    {
        // 不做任何事
    }
    else
    {
        // 若为全F，则在T0时刻发送上行信号
        if (turnOffLineSignalTimeString == TimeF)
        {
            turnOffLineSignalTime = GlobalData::currentDateTime();
        }
        else
        {
            turnOffLineSignalTime = QDateTime::fromString(turnOffLineSignalTimeString, ORIGINALFORMAT);
        }

        currentDateTime = GlobalData::currentDateTime();
        number = -1;
        while (turnOffLineSignalTime > currentDateTime)
        {
            // 如果退出就直接返回
            if (m_isExit)
                return;

            number++;
            if (number % 5 == 0)
            {
                QString info = QString("关上行流程还有%1秒开始执行").arg(currentDateTime.secsTo(turnOffLineSignalTime));
                notifyInfoLog(info);
            }
            QThread::sleep(1);
            currentDateTime = GlobalData::currentDateTime();
        }
        notifyInfoLog("关上行流程开始执行");

        // 测控基带停发上行
        if (taskTimeList.m_dataMap.contains(STTC))
        {
            //载波停发
            m_singleCommandHelper.packSingleCommand("Step_CK_TTC_5", packCommand, m_ckDeviceMap[STTC]);
            waitExecSuccess(packCommand);

            // S功放去电
            m_singleCommandHelper.packSingleCommand("Step_SHPA_Power_OFF", packCommand, m_hpaDeviceMap[STTC]);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(Skuo2))
        {
            //载波停发
            m_singleCommandHelper.packSingleCommand("Step_CK_S_5", packCommand, m_ckDeviceMap[Skuo2]);
            waitExecSuccess(packCommand);
            // S功放去电
            m_singleCommandHelper.packSingleCommand("Step_SHPA_Power_OFF", packCommand, m_hpaDeviceMap[Skuo2]);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(KaKuo2))
        {
            // 关基带载波
            m_singleCommandHelper.packSingleCommand("Step_CK_KA_5", packCommand, m_ckDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);

            // Ka测控功放负载
            m_singleCommandHelper.packSingleCommand("Step_HPA_5_1", packCommand, m_hpaDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);

            // 射频输出关 Ka测控功放
            m_singleCommandHelper.packSingleCommand("Step_HPA_4_1", packCommand, m_hpaDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);

            // 功放去电   Ka测控功放
            m_singleCommandHelper.packSingleCommand("Step_HPA_15_QD", packCommand, m_hpaDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);
        }

        if (taskTimeList.m_dataMap.contains(KaDS))
        {
            // Ka数传功放负载
            m_singleCommandHelper.packSingleCommand("Step_SCHPA_5_1", packCommand, m_hpaDeviceMap[KaDS]);
            waitExecSuccess(packCommand);
            //设置数传功放射频输出关 Ka数传功放
            m_singleCommandHelper.packSingleCommand("Step_SCHPA_4_1", packCommand, m_hpaDeviceMap[KaDS]);
            waitExecSuccess(packCommand);
            // 功放去电   Ka数传功放
            m_singleCommandHelper.packSingleCommand("Step_SCHPA_15_QD", packCommand, m_hpaDeviceMap[KaDS]);
            waitExecSuccess(packCommand);
        }

        // 测控基带去调遥控信号
        if (taskTimeList.m_dataMap.contains(STTC))
        {
            m_singleCommandHelper.packSingleCommand("Step_STTC_RemoteControl_OFF", packCommand, m_ckDeviceMap[STTC]);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(Skuo2))
        {
            m_singleCommandHelper.packSingleCommand("Step_CK_S_6_1", packCommand, m_ckDeviceMap[Skuo2]);
            waitExecSuccess(packCommand);
        }
        else if (taskTimeList.m_dataMap.contains(KaKuo2))
        {
            m_singleCommandHelper.packSingleCommand("Step_CK_KA_6_1", packCommand, m_ckDeviceMap[KaKuo2]);
            waitExecSuccess(packCommand);
        }

        // 控制功率下天线
        // 功放关闭   S功放

        //        // S功放去负载（A或B套去负载）
        //        cmd.getTaskPlanCmd(array, QString("Step_PAS_1_1"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID,
        //        m_runningCmdID); waitExecSuccess(array);

        notifyInfoLog("关上行流程完成");
    }

    QThread::sleep(1);
    notify("TrackEnd");
}
