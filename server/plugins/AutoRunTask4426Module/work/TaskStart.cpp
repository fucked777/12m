#include "TaskStart.h"
#include "BaseEvent.h"
#include "BaseHandler.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

// int TaskStart::typeId = qRegisterMetaType<TaskStart>();
TaskStart::TaskStart(QObject* parent)
    : BaseEvent(parent)
{
}

void TaskStart::doSomething()
{
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    auto taskStartDateTime = taskTimeList.m_task_start_time;
    auto currentDateTime = GlobalData::currentDateTime();
    while (taskStartDateTime > currentDateTime)
    {
        // 如果退出就直接返回
        if (isExit())
        {
            return;
        }

        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }
    notifyInfoLog("任务开始流程开始执行");
    VoiceAlarmPublish::publish("任务开始");

    // 基带开启存盘 送数
    deviceTaskStart(taskTimeList);
    // ACU 任务开始
    acuTaskStart(taskTimeList);
    // DTE 任务开始
    // dteTaskStart(taskTimeList);

    notifyInfoLog(QString("任务开始流程完成"));
    //QThread::sleep(1);
    notify("Calibration");
}

void TaskStart::deviceTaskStart(const TaskTimeList& taskTimeList)
{
    auto& configMacroData = taskTimeList.manualMessage.configMacroData;
    /* 测控任务开始 */
    ckTaskStart(taskTimeList);

    // X低速
    if (configMacroData.existWorkMode(XDS))
    {
        DeviceID deviceID;
        if (!configMacroData.getDSJDDeviceID(XDS, deviceID))
        {
            notifyErrorLog("未查找到当前任务的低速基带");
            return;
        }

        //打开低速基带送数
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Start_X", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开低速基带存盘
        // m_singleCommandHelper.packSingleCommand("StepDSJDCP_Start_X_1", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
        // m_singleCommandHelper.packSingleCommand("StepDSJDCP_Start_X_2", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
    }
    // Ka低速
    if (configMacroData.existWorkMode(KaDS))
    {
        DeviceID deviceID;
        if (!configMacroData.getDSJDDeviceID(KaDS, deviceID))
        {
            notifyErrorLog("未查找到当前任务的低速基带");
            return;
        }

        //打开低速基带送数
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Start_Ka", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开低速基带存盘
        // m_singleCommandHelper.packSingleCommand("StepDSJDCP_Start_Ka_1", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
        // m_singleCommandHelper.packSingleCommand("StepDSJDCP_Start_Ka_2", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
    }

    // Ka高速
    if (configMacroData.existWorkMode(KaGS))
    {
        DeviceID deviceID;
        if (!configMacroData.getGSJDDeviceID(KaGS, deviceID))
        {
            notifyErrorLog("未查找到当前任务的高速基带");
            return;
        }

        //打开高速基带送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开高速基带存盘
        // m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_1", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
        // m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_2", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
        // m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_1", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
        // m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_2", m_packCommand, deviceID);
        // waitExecSuccess(m_packCommand);
    }
}
void TaskStart::ckTaskStart(const TaskTimeList& taskTimeList)
{
    QSet<SystemWorkMode> workModeSet;
    for (auto iter = taskTimeList.m_dataMap.begin(); iter != taskTimeList.m_dataMap.end(); ++iter)
    {
        auto workMode = SystemWorkMode(iter.key());
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            workModeSet << workMode;
        }
    }
    // 无测控模式
    if (workModeSet.isEmpty())
    {
        return;
    }

    // 获取测控基带ID
    auto result = BaseHandler::getCKJDControlInfo(taskTimeList.manualMessage.configMacroData, *workModeSet.begin());
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        notifyErrorLog(msg);
    }
    if (!result)
    {
        return;
    }
    auto ckjdInfo = result.value();
    auto& modeExist = taskTimeList.modeExist;

    // S标准TTC
    if (modeExist.skuo2Mode && modeExist.kakuo2Mode)
    {
        //送数控制
        auto size = taskTimeList.m_dataMap[Skuo2].size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);

            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterSlave);
            waitExecSuccess(m_packCommand);
        }

        size = taskTimeList.m_dataMap[KaKuo2].size();
        auto afterMaster2 = BaseHandler::getEqModeCKDeviceID(ckjdInfo.afterMaster);
        auto afterSlave2 = BaseHandler::getEqModeCKDeviceID(ckjdInfo.afterSlave);

        for (auto i = 1; i < size; i++)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_KaKuo2_%1").arg(i), m_packCommand, afterMaster2);
            waitExecSuccess(m_packCommand);

            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), m_packCommand, afterMaster2);
            waitExecSuccess(m_packCommand);
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), m_packCommand, afterSlave2);
            waitExecSuccess(m_packCommand);
        }
    }
    // S扩二
    else if (modeExist.skuo2Mode)
    {
        //送数控制
        auto size = taskTimeList.m_dataMap[Skuo2].size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);

            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), m_packCommand, ckjdInfo.afterSlave);
            waitExecSuccess(m_packCommand);
        }
    }
    // Ka扩二
    else if (modeExist.kakuo2Mode)
    {
        //送数控制
        auto size = taskTimeList.m_dataMap[KaKuo2].size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_KaKuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);

            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), m_packCommand, ckjdInfo.afterMaster);
            waitExecSuccess(m_packCommand);
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), m_packCommand, ckjdInfo.afterSlave);
            waitExecSuccess(m_packCommand);
        }
    }
    else if (modeExist.sttcMode)
    {
        //打开基带送数
        m_singleCommandHelper.packSingleCommand("StepCKJDSS_Start_TTC", m_packCommand, ckjdInfo.afterMaster);
        waitExecSuccess(m_packCommand);

        //打开基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJDCP_Start_TTC", m_packCommand, ckjdInfo.afterMaster);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJDCP_Start_TTC", m_packCommand, ckjdInfo.afterSlave);
        waitExecSuccess(m_packCommand);
    }
    // 扩跳
    else if (modeExist.sktMode)
    {
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_SKT"), m_packCommand, ckjdInfo.afterMaster);
        waitExecSuccess(m_packCommand);

        //默认打开存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_SKT"), m_packCommand, ckjdInfo.afterMaster);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_SKT"), m_packCommand, ckjdInfo.afterSlave);
        waitExecSuccess(m_packCommand);
    }
}
void TaskStart::acuTaskStart(const TaskTimeList& taskTimeList)
{
    notifyInfoLog(QString("ACU任务开始"));
    // 控制ACU进入程序引导 不需要回复
    m_singleCommandHelper.packSingleCommand("Step_ACU_ProgramGuide", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand, 0);
    // ACU 开启送数给DTE
    m_singleCommandHelper.packSingleCommand("Step_ACU_2_DET_START", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand);
}

// void TaskStart::dteTaskStart(const TaskTimeList& taskTimeList)
//{
//    // 无测控模式
//    // if (!taskTimeList.existCKMode)
//    // {
//    //     notifyInfoLog("当前无测控任务跳过DTE任务下发");
//    //     return;
//    // }

//    //根据任务代号获取任务标识，天线标识， 向DTE下发瞬根申请命令
//    // QVariantMap dteSunGenParamMap;
//    // dteSunGenParamMap["MID"] = taskTimeList.manualMessage.masterTaskBz;  // 任务标识
//    // dteSunGenParamMap["UAC"] = taskTimeList.manualMessage.txUACAddr;     // 天线标识
//    // 发送DTE_任务开始
//    // m_singleCommandHelper.packSingleCommand("Step_DTE_TASK_START", m_packCommand, dteSunGenParamMap);
//    // waitExecSuccess(m_packCommand);

//    // auto & taskPlanData = taskTimeList.taskPlanData;
//    //    QList<SystemWorkMode> workModeList={STTC , Skuo2 , KaKuo2 , SKT};
//    //    for(auto&workMode:workModeList)
//    //    {
//    //        for (auto item : taskTimeList.m_dataMap[workMode])
//    //        {
//    //            if (item.isMaster)
//    //            {
//    //                for(auto&center : datatransCenter)
//    //                {
//    //                    dteSunGenParamMap["CenterID"]=center;
//    //                    //开启DTE送数
//    //                    //m_singleCommandHelper.packSingleCommand("Step_DTE_SENDDATA_START", m_packCommand,dteSunGenParamMap);
//    //                    //waitExecSuccess(m_packCommand);

//    //                    //打开送数开关
//    //                    m_singleCommandHelper.packSingleCommand("Step_DTE_CENTER_SENDDATA_START", m_packCommand,dteSunGenParamMap);
//    //                    waitExecSuccess(m_packCommand);
//    //                }

//    //            }
//    //        }
//    //    }
//}
