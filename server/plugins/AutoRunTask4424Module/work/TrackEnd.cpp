#include "TrackEnd.h"
#include "GlobalData.h"
#include "TaskGlobalInfo.h"

// int TrackEnd::typeId = qRegisterMetaType<TrackEnd>();
TrackEnd::TrackEnd(QObject* parent)
    : BaseEvent(parent)
{
}

void TrackEnd::doSomething()
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
     *  T6 跟踪结束时间
     * 系统监控控制ACU、测控基带、数传基带、DTE送数关和存盘关
     ***/
    auto trackEndTime = taskTimeList.m_track_end_time;
    auto currentDateTime = GlobalData::currentDateTime();
    int number = -1;
    QString info = QString("跟踪结束流程还有%1秒开始执行").arg(currentDateTime.secsTo(trackEndTime));
    notifyInfoLog(info);
    while (trackEndTime > currentDateTime)
    {
        // 如果退出就直接返回
        if (m_isExit)
            return;

        number++;

        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }
    notifyInfoLog("跟踪结束流程开始执行");

    // DTE停止送数命令 对应中心数据发送控制
    QMap<QString, QVariant> dteTaskLoadParamMap;
    dteTaskLoadParamMap["CenterID"] = 0x17BE;
    m_singleCommandHelper.packSingleCommand("Step_DIC_12_END", packCommand, m_hpaDeviceMap[KaDS], dteTaskLoadParamMap);
    waitExecSuccess(packCommand);

    // DTE存盘关 （暂无）
    // ？？？？？

    notifyInfoLog("跟踪结束流程完成");

    /***
     * T7 任务结束时间
     * 1.系统监控向测控基带发送测控结束命令，向ACU发送停止角度数据发送命令，向DTE发送停止送数命令
     * 2.ACU判断信号丢失不满足跟踪条件，控制天线退出自跟踪，天线方位、俯仰、第三轴归零，转待机
     ***/

    auto taskEndTime = taskTimeList.m_task_end_time;
    currentDateTime = GlobalData::currentDateTime();
    info = QString("任务结束流程还有%1秒开始执行").arg(currentDateTime.secsTo(taskEndTime));
    notifyInfoLog(info);
    while (taskEndTime > currentDateTime)
    {
        QThread::sleep(1);
        currentDateTime = GlobalData::currentDateTime();
    }
    notifyInfoLog("任务结束流程开始执行");

    deviceTaskEnd(taskTimeList);
    acuTaskEnd(taskTimeList);
    dteTaskEnd(taskTimeList);

    notifyInfoLog("任务结束流程完成");
    QThread::sleep(1);
    notify("PostProcessing");
}

void TrackEnd::deviceTaskEnd(TaskTimeList taskTimeList)
{
    QMap<int, DeviceID> m_ckDeviceMap = taskTimeList.m_ckDeviceMap;      //测控模式下的测控基带信息
    QMap<int, DeviceID> m_ckQdDeviceMap = taskTimeList.m_ckQdDeviceMap;  //测控前端信息
    QMap<int, DeviceID> m_gzDeviceMap = taskTimeList.m_gzDeviceMap;      //跟踪基带信息
    QMap<int, DeviceID> m_gzQdDeviceMap = taskTimeList.m_gzQdDeviceMap;  //跟踪前端信息
    QMap<int, DeviceID> m_dsDeviceMap = taskTimeList.m_dsDeviceMap;      //低速基带信息
    QMap<int, DeviceID> m_gsDeviceMap = taskTimeList.m_gsDeviceMap;      //高速基带信息
    QMap<int, DeviceID> m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;    //功放的信息

    PackCommand packCommand;
    // S标准TTC
    if (taskTimeList.m_dataMap.contains(STTC))
    {
        // 基带的工作方式设置为任务方式
        DeviceID deviceID = m_ckDeviceMap[STTC];

        //基带停止送数
        m_singleCommandHelper.packSingleCommand("StepCKJDSS_Stop_TTC", packCommand, deviceID);
        waitExecSuccess(packCommand);
        //停止基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJDCP_Stop_TTC", packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // S扩二
    if (taskTimeList.m_dataMap.contains(Skuo2))
    {
        // 基带的工作方式设置为任务方式
        DeviceID deviceID = m_ckDeviceMap[Skuo2];
        //送数控制
        auto size = taskTimeList.m_dataMap[Skuo2].size();
        for (auto i = 1; i < size; i++)
        {
            //有多少通道就多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_Skuo2_%1").arg(i), packCommand, deviceID);
            waitExecSuccess(packCommand);
        }

        //默认存盘

        for (auto i = 1; i < size; i++)
        {
            //有多少通道就多少通道的送数
            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_Skuo2_%1").arg(i), packCommand, deviceID);
            waitExecSuccess(packCommand);
        }
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // Ka扩二
    //    if (taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        // 基带的工作方式设置为任务方式
    //        DeviceID deviceID = m_ckDeviceMap[KaKuo2];
    //        //送数控制
    //        auto size = taskTimeList.m_dataMap[KaKuo2].size();
    //        for (auto i = 1; i < size; i++)
    //        {
    //            //有多少通道就多少通道的送数
    //            m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_KaKuo2_%1").arg(i), packCommand, deviceID);
    //            waitExecSuccess(packCommand);
    //        }
    //        //默认存盘
    //        for (auto i = 1; i < size; i++)
    //        {
    //            //有多少通道就多少通道的送数
    //            m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_KaKuo2_%1").arg(i), packCommand, deviceID);
    //            waitExecSuccess(packCommand);
    //        }
    //        //基带下天线
    //        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //    }

    // 一体化上面级
    if (taskTimeList.m_dataMap.contains(SYTHSMJ))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHSMJ];
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHSMJ"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHSMJ"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星
    if (taskTimeList.m_dataMap.contains(SYTHWX))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHWX];
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHWX"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHWX"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率
    if (taskTimeList.m_dataMap.contains(SYTHGML))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHGML];
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHGML"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHGML"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二共载波
    if (taskTimeList.m_dataMap.contains(SYTHSMJK2GZB))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHSMJK2GZB];
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHSMJK2GZB_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHSMJK2GZB_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHSMJK2GZB_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHSMJK2GZB_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化上面级+扩二不共载波
    if (taskTimeList.m_dataMap.contains(SYTHSMJK2BGZB))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHSMJK2BGZB];
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHSMJK2BGZB_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHSMJK2BGZB_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHSMJK2BGZB_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHSMJK2BGZB_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化卫星+扩二
    if (taskTimeList.m_dataMap.contains(SYTHWXSK2))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHWXSK2];
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHWXSK2_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHWXSK2_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHWXSK2_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHWXSK2_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 一体化高码率+扩二
    if (taskTimeList.m_dataMap.contains(SYTHGMLSK2))
    {
        DeviceID deviceID = m_ckDeviceMap[SYTHGMLSK2];
        //送数控制 ，目前没区分一体化送数还是扩频送数
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHGMLSK2_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SYTHGMLSK2_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘 ，目前没区分一体化存盘还是扩频存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHGMLSK2_YTH"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SYTHGMLSK2_KP"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
    // 扩跳
    if (taskTimeList.m_dataMap.contains(SKT))
    {
        DeviceID deviceID = m_ckDeviceMap[SKT];
        //送数控制
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDSS_Stop_SKT"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //默认存盘
        m_singleCommandHelper.packSingleCommand(QString("StepCKJDCP_Stop_SKT"), packCommand, deviceID);
        waitExecSuccess(packCommand);
        //基带下天线
        m_singleCommandHelper.packSingleCommand("Step_GLSTX_STOP", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }

    // X低速
    //    if (taskTimeList.m_dataMap.contains(XDS))
    //    {
    //        //设置任务方式
    //        DeviceID deviceID = m_dsDeviceMap[XDS];
    //        //低速基带停止送数
    //        m_singleCommandHelper.packSingleCommand("StepDSJDSSStop_X", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //        //低速停止基带存盘
    //        m_singleCommandHelper.packSingleCommand("StepDSJDCPStop_X_1", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("StepDSJDCPStop_X_2", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //    }
    // Ka低速
    //    if (taskTimeList.m_dataMap.contains(KaDS))
    //    {
    //        //设置任务方式
    //        DeviceID deviceID = m_dsDeviceMap[KaDS];
    //        //低速基带停止送数
    //        m_singleCommandHelper.packSingleCommand("StepDSJDSSStop_Ka", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //        //低速停止基带存盘
    //        m_singleCommandHelper.packSingleCommand("StepDSJDCPStop_Ka_1", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("StepDSJDCPStop_Ka_2", packCommand, deviceID);
    //        waitExecSuccess(packCommand);
    //    }
    // X高速
    if (taskTimeList.m_dataMap.contains(XGS))
    {
        //设置任务方式
        DeviceID deviceID = m_gsDeviceMap[XGS];
        //高速基带停止送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSSStop_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSSStop_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
        //高速停止基带存盘
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_1_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_1_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_2_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_2_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }

    // Ka高速
    if (taskTimeList.m_dataMap.contains(KaGS))
    {
        //设置任务方式
        DeviceID deviceID = m_gsDeviceMap[KaGS];
        //高速基带停止送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSSStop_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSSStop_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
        //高速停止基带存盘
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_1_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_1_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_2_1", packCommand, deviceID);
        waitExecSuccess(packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCPStop_2_2", packCommand, deviceID);
        waitExecSuccess(packCommand);
    }
}

void TrackEnd::acuTaskEnd(TaskTimeList taskTimeList)
{
    PackCommand packCommand;
    // ACU送数开关、存盘均为状态
    DeviceID deviceACUMaster;  //主用的ACU
    auto onlineACUResult = getOnlineACU();
    if (!onlineACUResult)
    {
        notifyErrorLog(onlineACUResult.msg());
        // notify("error");
        // return;
        onlineACUResult = Optional<DeviceID>(DeviceID(ACU_A)); /* 有设备或者正常调试以后 删出 此行 */
    }
    deviceACUMaster = onlineACUResult.value();

    notifyInfoLog(QString("控制ACU进入程序引导"));

    // ACU 停止送数给DTE
    m_singleCommandHelper.packSingleCommand("Step_ACU_2_DET_END", packCommand, deviceACUMaster);
    waitExecSuccess(packCommand);
}

void TrackEnd::dteTaskEnd(TaskTimeList taskTimeList)
{
    //控制DTE任务结束
    //    PackCommand packCommand;
    //    DeviceID dteADeviceID(DTE_A);
    //    DeviceID dteBDeviceID(DTE_B);

    //    DeviceID deviceDTEMaster;  //主用的ACU

    //    if (!GlobalData::getDeviceOnline(dteADeviceID) && !GlobalData::getDeviceOnline(dteBDeviceID))
    //    {
    //        //如果ACU设备同时离线 ，上报ACU异常
    //        notifyErrorLog("ACU主备设备离线，任务失败");
    //        // notify("error");
    //        // return;
    //    }
    //    else
    //    {
    //        //如果ACU_A在线
    //        if (!GlobalData::getDeviceOnline(dteADeviceID))
    //        {
    //            deviceDTEMaster = dteADeviceID;
    //        }
    //        //如果ACU_B在线
    //        else if (!GlobalData::getDeviceOnline(dteBDeviceID))
    //        {
    //            deviceDTEMaster = dteBDeviceID;
    //        }
    //    }
    //    if (deviceDTEMaster.isValid())
    //    {
    //        for (auto workMode = STTC; workMode <= SKT;)
    //        {
    //            if (workMode != XDS || workMode != XGS || workMode != KaDS || workMode != KaGS)
    //            {
    //                for (auto item : taskTimeList.m_dataMap[workMode])
    //                {
    //                    if (item.isMaster)
    //                    {
    //                        //根据任务代号获取任务标识，天线标识， 向DTE下发瞬根申请命令
    //                        QMap<QString, QVariant> dteSunGenParamMap;
    //                        dteSunGenParamMap["MID"] = item.m_taskBz;  // 任务标识
    //                        dteSunGenParamMap["UAC"] = 0x66060106;     // 天线标识
    //                        // 发送DTE_任务开始
    //                        m_singleCommandHelper.packSingleCommand("Step_DTE_TASKStop", packCommand, deviceDTEMaster);
    //                        waitExecSuccess(packCommand);
    //                    }
    //                }
    //            }
    //            workMode = SystemWorkMode((int)workMode + 1);
    //        }
    //    }
}
