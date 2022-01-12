#include "GoUp.h"
#include "MessagePublish.h"
#include "TaskGlobalInfo.h"
#include "UpLinkHandler.h"
#include <QDebug>
#include <QThread>
// int GoUp::typeId = qRegisterMetaType<GoUp>();
GoUp::GoUp(QObject* parent)
    : BaseEvent(parent)
{
}

void GoUp::doSomething()
{
    taskMachine->updateStatus("GoUp", TaskStepStatus::Running);
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    SystemWorkMode ckUpMode{ SystemWorkMode::NotDefine };  // 测控上行的模式
    bool ckupYK = false;
    bool ckupCG = false;
    SystemWorkMode scUpMode{ SystemWorkMode::NotDefine };  // 数传上行的模式 数传只能发遥控

    /*
     * 如果有多个发上行的冲突取第一个就行
     * 测控和低速数传不冲突
     * 取第一个就行
     */
    for (auto iter = taskTimeList.m_dataMap.begin(); iter != taskTimeList.m_dataMap.end(); ++iter)
    {
        auto workMode = SystemWorkMode(iter.key());
        auto& entityList = iter.value();
        // 点频
        for (auto& entity : entityList)
        {
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
            {
                if ((entity.m_taskType_yk || entity.m_taskType_cg) && (!(ckupYK || ckupCG)))
                {
                    ckupYK = entity.m_taskType_yk;
                    ckupCG = entity.m_taskType_cg;
                    ckUpMode = workMode;
                    auto msg = QString("当前测控发上行的模式是: %1 遥控: %2 测轨: %3")
                                   .arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode), ckupYK ? "开" : "关", ckupYK ? "开" : "关");
                    notifyInfoLog(msg);
                    continue;
                }
                if ((ckupYK && entity.m_taskType_yk) || (ckupCG && entity.m_taskType_cg))
                {
                    auto msg = QString("当前测控已有发上行的模式：%1  冲突的模式是: %2")
                                   .arg(SystemWorkModeHelper::systemWorkModeToDesc(ckUpMode), SystemWorkModeHelper::systemWorkModeToDesc(workMode));
                    notifyWarningLog(msg);
                }
            }
            else if (workMode == KaDS)
            {
                if (scUpMode != SystemWorkMode::NotDefine)
                {
                    auto msg = QString("当前数传已有发上行的模式：%1  冲突的模式是: %2")
                                   .arg(SystemWorkModeHelper::systemWorkModeToDesc(scUpMode), SystemWorkModeHelper::systemWorkModeToDesc(workMode));
                    notifyWarningLog(msg);
                }
                if (entity.m_taskType_yk)
                {
                    scUpMode = workMode;
                    auto msg = QString("当前数传发上行的模式是: %1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
                    notifyInfoLog(msg);
                }
            }
        }
    }

    if (ckUpMode == SystemWorkMode::NotDefine && scUpMode == SystemWorkMode::NotDefine)
    {
        notifyInfoLog("当前未选择遥控,测轨的模式信息跳过发上行");
        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
        //QThread::sleep(1);
        notify("TrackEnd");
        return;
    }

    /***
     * 开上行 开功放 开载波 有测轨开测距
     * 开遥控 遥控加调
     * 关遥控 遥控去调
     * 关上行 关功放 关载波--->全关
     *
     * T4 开上行载波时间
     * 1.系统监控根据设备工作计划中的开上行信号时间控制功放信号上天线
     * 2.系统监控根据设备工作计划中遥控开始时间发送遥控信号
     ***/

    QDateTime currentDateTime;
    /*** 1.系统监控根据设备工作计划中的开上行信号时间控制功放信号上天线 ***/
    auto onUpLinkSignalTimeString = taskTimeList.m_on_uplink_signal_time;
    QDateTime onUpLinkSignalTime;
    // 若设备工作计划中该项为全0，则不发上行信号
    if (onUpLinkSignalTimeString == TimeZero)
    {
        notifyInfoLog("开上行信号时间为全为0，不发上行信号");
        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
    }
//    20211215 颜 要求改为常发
//    // 若为全F，系统决定默认不发
//    else if (onUpLinkSignalTimeString == TimeF)
//    {
//        notifyInfoLog("开上行信号时间为全为F，系统决定默认不发上行信号");
//        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
//    }
    else
    {
        if(onUpLinkSignalTimeString != TimeF)
        {
            onUpLinkSignalTime = QDateTime::fromString(onUpLinkSignalTimeString, ORIGINALFORMAT);
            currentDateTime = GlobalData::currentDateTime();
            int number = -1;
            while (onUpLinkSignalTime >= currentDateTime)
            {
                // 如果退出就直接返回
                if (isExit())
                {
                    return;
                }

                number++;
                QThread::msleep(100);
                currentDateTime = GlobalData::currentDateTime();
            }
        }

        notifyInfoLog("发上行流程开始执行");
        VoiceAlarmPublish::publish("发上行");
        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableHpa(EnableMode::Enable);
        upLinkHandler.setEnableJD(EnableMode::Enable);
        upLinkHandler.setEnableQD(EnableMode::Enable);

        if (ckUpMode != SystemWorkMode::NotDefine && scUpMode != SystemWorkMode::NotDefine)
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::All);
        }
        else if (ckUpMode != SystemWorkMode::NotDefine)
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::CK);
        }
        else if (scUpMode != SystemWorkMode::NotDefine)
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::SC);
        }
        else
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::Ignore);
        }

        upLinkHandler.setManualMessage(taskTimeList.manualMessage);
        upLinkHandler.setLinkLine(taskTimeList.manualMessage.linkLineMap.value(ckUpMode));
        upLinkHandler.setQDSendSource(CKQDSendSource::JD);
        upLinkHandler.setRFOutputMode(RFOutputMode::TX);
        upLinkHandler.setAdd(JDOutPut::CarrierRangAdd);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &GoUp::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &GoUp::notifyInfoLog);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &GoUp::notifyWarningLog);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &GoUp::notifyErrorLog);
        connect(&upLinkHandler, &UpLinkHandler::signalSpecificTipsMsg, this, &GoUp::notifySpecificTipsLog);

        upLinkHandler.handle();
        taskMachine->updateStatus("GoUp", TaskStepStatus::Finish);
        notifyInfoLog("发上行流程完成");
    }

    /*** 2.系统监控根据设备工作计划中遥控开始时间发送遥控信号 ***/
    auto remoteControlStartTimeString = taskTimeList.m_remote_start_time;
    QDateTime remoteControlStartTime;
    // 若设备工作计划中该项为全0，则不发上行信号
    if (remoteControlStartTimeString == TimeZero)
    {
        notifyInfoLog("遥控开始时间为全为0，不发遥控信号");
        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
    }
    //    20211215 颜 要求改为常发
    //    else if (remoteControlStartTimeString == TimeF)
    //    {
    //        notifyInfoLog("遥控开始时间为全为F，系统决定默认不发遥控信号");
    //        taskMachine->updateStatus("GoUp", TaskStepStatus::Continue);
    //    }
    else
    {
        if(remoteControlStartTimeString != TimeF)
        {
            remoteControlStartTime = QDateTime::fromString(remoteControlStartTimeString, ORIGINALFORMAT);

            currentDateTime = GlobalData::currentDateTime();
            int number = -1;
            while (remoteControlStartTime > currentDateTime)
            {
                // 如果退出就直接返回
                if (isExit())
                {
                    return;
                }

                number++;
                QThread::sleep(1);
                currentDateTime = GlobalData::currentDateTime();
            }
        }

        notifyInfoLog("发送遥控信号开始执行");
        VoiceAlarmPublish::publish("发遥控");

        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableHpa(EnableMode::Ignore);
        upLinkHandler.setEnableJD(EnableMode::Enable);
        upLinkHandler.setEnableQD(EnableMode::Ignore);

        if (ckUpMode != SystemWorkMode::NotDefine)
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::CK);
        }
        else
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::Ignore);
        }

        upLinkHandler.setManualMessage(taskTimeList.manualMessage);
        upLinkHandler.setLinkLine(taskTimeList.manualMessage.linkLineMap.value(ckUpMode));
        upLinkHandler.setAdd(JDOutPut::CarrierRemoteAdd);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &GoUp::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &GoUp::notifyInfoLog);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &GoUp::notifyWarningLog);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &GoUp::notifyErrorLog);
        connect(&upLinkHandler, &UpLinkHandler::signalSpecificTipsMsg, this, &GoUp::notifySpecificTipsLog);

        upLinkHandler.handle();
        taskMachine->updateStatus("GoUp", TaskStepStatus::Finish);
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
            turnOffLineSignalTime = taskTimeList.m_track_end_time;  //全F关上行时间为跟踪结束时间
        }
        else
        {
            turnOffLineSignalTime = QDateTime::fromString(turnOffLineSignalTimeString, "yyyy-MM-dd HH:mm:ss");
        }

        currentDateTime = GlobalData::currentDateTime();
        int number = -1;
        while (turnOffLineSignalTime > currentDateTime)
        {
            // 如果退出就直接返回
            if (isExit())
            {
                return;
            }

            number++;
            QThread::sleep(1);
            currentDateTime = GlobalData::currentDateTime();
        }
        notifyInfoLog("关上行流程开始执行");

        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableHpa(EnableMode::Disable);
        upLinkHandler.setEnableJD(EnableMode::Disable);
        upLinkHandler.setEnableQD(EnableMode::Ignore);
        upLinkHandler.setIgnoreOtherDevice(true);
        upLinkHandler.setIgnoreXLDevice(true);
        upLinkHandler.enableControl(UpLinkDeviceControl::All);
        upLinkHandler.setManualMessage(taskTimeList.manualMessage);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &GoUp::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &GoUp::notifyInfoLog);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &GoUp::notifyWarningLog);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &GoUp::notifyErrorLog);
        connect(&upLinkHandler, &UpLinkHandler::signalSpecificTipsMsg, this, &GoUp::notifySpecificTipsLog);

        upLinkHandler.handle();
        VoiceAlarmPublish::publish("发上行结束");

        notifyInfoLog("关上行流程完成");
    }

    //QThread::sleep(1);
    notify("TrackEnd");
}
