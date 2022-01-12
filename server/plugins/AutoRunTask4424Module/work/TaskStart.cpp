#include "TaskStart.h"
#include "BaseEvent.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

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
        if (m_isExit)
            return;

        QThread::sleep(800);
        currentDateTime = GlobalData::currentDateTime();
    }
    notifyInfoLog("任务开始流程开始执行");
    VoiceAlarmPublish::publish("任务开始");

    // 基带开启存盘 送数
    deviceTaskStart(taskTimeList);
    // ACU 任务开始
    acuTaskStart(taskTimeList);
    // DTE 任务开始
    dteTaskStart(taskTimeList);

    notifyInfoLog(QString("任务开始流程完成"));
    QThread::sleep(1);
    notify("Calibration");
}

void TaskStart::deviceTaskStart(TaskTimeList taskTimeList)
{
    QMap<int, DeviceID> ckjdDeviceMap = taskTimeList.m_ckDeviceMap;  // 测控基带信息
    QMap<int, DeviceID> gsjdDeviceMap = taskTimeList.m_gsDeviceMap;  // 高速基带信息

    // S标准TTC
    if (taskTimeList.m_dataMap.contains(STTC))
    {
        // 基带的工作方式设置为任务方式
        DeviceID deviceID = ckjdDeviceMap[STTC];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, STTC);
        waitExecSuccess(m_packCommand);

        // 打开基带送数
        m_singleCommandHelper.packSingleCommand("StepCKJD_STTC_SendData_Start", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        // 打开基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJD_STTC_SaveDisk_Start", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // S扩二
    if (taskTimeList.m_dataMap.contains(Skuo2))
    {
        // 基带的工作方式设置为任务方式
        DeviceID deviceID = ckjdDeviceMap[Skuo2];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, Skuo2);
        waitExecSuccess(m_packCommand);
        //送数控制
        auto size = taskTimeList.m_dataMap[Skuo2].size();
        for (auto i = 1; i < size; i++)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SK2_SendData_Start_%1").arg(i), m_packCommand, deviceID);
            waitExecSuccess(m_packCommand);
        }

        //默认打开存盘
        for (auto i = 1; i < size; i++)
        {
            //有多少通道就打开多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SK2_SaveDisk_Start_%1").arg(i), m_packCommand, deviceID);
            waitExecSuccess(m_packCommand);
        }
    }
    // 一体化上面级
    if (taskTimeList.m_dataMap.contains(SYTHSMJ))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHSMJ];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHSMJ);
        waitExecSuccess(m_packCommand);
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJ_SendData_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJ_SaveDisk_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化卫星
    if (taskTimeList.m_dataMap.contains(SYTHWX))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHWX];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHWX);
        waitExecSuccess(m_packCommand);
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWX_SendData_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWX_SaveDisk_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化高码率
    if (taskTimeList.m_dataMap.contains(SYTHGML))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHGML];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHGML);
        waitExecSuccess(m_packCommand);
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGML_SendData_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGML_SaveDisk_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化上面级+扩二共载波
    if (taskTimeList.m_dataMap.contains(SYTHSMJK2GZB))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHSMJK2GZB];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHSMJK2GZB);
        waitExecSuccess(m_packCommand);
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2GZB_SendData_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2GZB_SendData_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2BGZB_SaveDisk_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2BGZB_SaveDisk_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化上面级+扩二不共载波
    if (taskTimeList.m_dataMap.contains(SYTHSMJK2BGZB))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHSMJK2BGZB];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHSMJK2BGZB);
        waitExecSuccess(m_packCommand);
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2BGZB_SendData_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2BGZB_SendData_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2GZB_SaveDisk_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHSMJK2GZB_SaveDisk_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化卫星+扩二
    if (taskTimeList.m_dataMap.contains(SYTHWXSK2))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHWXSK2];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHWXSK2);
        waitExecSuccess(m_packCommand);
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWXSK2_SendData_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWXSK2_SendData_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWXSK2_SaveDisk_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHWXSK2_SaveDisk_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 一体化高码率+扩二
    if (taskTimeList.m_dataMap.contains(SYTHGMLSK2))
    {
        DeviceID deviceID = ckjdDeviceMap[SYTHGMLSK2];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SYTHGMLSK2);
        waitExecSuccess(m_packCommand);
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGMLSK2_SendData_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGMLSK2_SendData_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGMLSK2_SaveDisk_Start_YTH"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SYTHGMLSK2_SaveDisk_Start_KP"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // 扩跳
    if (taskTimeList.m_dataMap.contains(SKT))
    {
        DeviceID deviceID = ckjdDeviceMap[SKT];
        m_singleCommandHelper.packSingleCommand("StepCKJD_RWFS", m_packCommand, deviceID, SKT);
        waitExecSuccess(m_packCommand);
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SKT_SendData_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //默认打开存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJD_SKT_SaveDisk_Start"), m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
    // X高速
    if (taskTimeList.m_dataMap.contains(XGS))
    {
        //设置任务方式
        DeviceID deviceID = gsjdDeviceMap[XGS];
        m_singleCommandHelper.packSingleCommand("StepXGS_RWFS", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开高速基带送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开高速基带存盘
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }

    // Ka高速
    if (taskTimeList.m_dataMap.contains(KaGS))
    {
        //设置任务方式
        DeviceID deviceID = gsjdDeviceMap[KaGS];
        m_singleCommandHelper.packSingleCommand("StepKaGS_RWFS", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开高速基带送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Start_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        //打开高速基带存盘
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_1_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_1", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Start_2_2", m_packCommand, deviceID);
        waitExecSuccess(m_packCommand);
    }
}

void TaskStart::acuTaskStart(TaskTimeList taskTimeList)
{
    // ACU送数开关、存盘均为打开状态
    notifyInfoLog(QString("控制ACU进入程序引导"));
    // 控制ACU进入程序引导
    // m_singleCommandHelper.packSingleCommand("Step_AC_ProgramGuide", m_packCommand, taskTimeList.onlineACU);
    // waitExecSuccess(m_packCommand);
    // ACU 开启送数给DTE
    m_singleCommandHelper.packSingleCommand("Step_ACU_2_DET_START", m_packCommand, taskTimeList.onlineACU);
    waitExecSuccess(m_packCommand);
}

void TaskStart::dteTaskStart(TaskTimeList taskTimeList)
{
    //控制DTE任务开始
    if (!dteOnlineCheck())
    {
        notifyErrorLog("未能检测到DTE,配置文件错误者设备离线");
        VoiceAlarmPublish::publish("DTE状态获取失败");
        return;
    }
    //根据任务代号获取任务标识，天线标识， 向DTE下发瞬根申请命令
    QVariantMap dteSunGenParamMap;
    dteSunGenParamMap["MID"] = taskTimeList.masterTaskBz;  // 任务标识
    dteSunGenParamMap["UAC"] = taskTimeList.txUACAddr;     // 天线标识
    // 发送DTE_任务开始
    m_singleCommandHelper.packSingleCommand("Step_DTE_TASK_START", m_packCommand, dteSunGenParamMap);
    waitExecSuccess(m_packCommand);

    auto& taskPlanData = taskTimeList.taskPlanData;
    // 数传中心
    QSet<QString> datatransCenter;
    for (auto& item : taskPlanData.dataTransWorkTasks)
    {
        datatransCenter << item.m_datatrans_center;
    }

    QList<SystemWorkMode> workModeList = SystemWorkModeHelper::getMeasureContrlWorkMode();
    for (auto& workMode : workModeList)
    {
        for (auto item : taskTimeList.m_dataMap[workMode])
        {
            if (item.isMaster)
            {
                for (auto& center : datatransCenter)
                {
                    dteSunGenParamMap["CenterID"] = center;
                    // 开启DTE送数
                    m_singleCommandHelper.packSingleCommand("Step_DTE_SENDDATA_START", m_packCommand, dteSunGenParamMap);
                    waitExecSuccess(m_packCommand);

                    // 打开送数开关
                    m_singleCommandHelper.packSingleCommand("Step_DTE_CENTER_SENDDATA_START", m_packCommand, dteSunGenParamMap);
                    waitExecSuccess(m_packCommand);
                }
            }
        }
    }
}
