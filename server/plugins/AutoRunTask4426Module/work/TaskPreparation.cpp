#include "TaskPreparation.h"

#include "ACUParamMacroHandler.h"
#include "AutorunPolicyMessageDefine.h"
#include "BaseHandler.h"
#include "BusinessMacroCommon.h"
#include "ControlFlowHandler.h"
#include "GlobalData.h"
#include "LinkConfigHelper.h"
#include "MessagePublish.h"
#include "ParamMacroDispatcher.h"
#include "PlanRunMessageDefine.h"
#include "RedisHelper.h"
#include "ResourceReleaseDispatcher.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <array>
// int TaskPreparation::typeId = qRegisterMetaType<TaskPreparation>();
TaskPreparation::TaskPreparation(QObject* parent)
    : BaseEvent(parent)
{
}

void TaskPreparation::doSomething()
{
    //处理任务类型时标 BJ 占用的情况 ZY
    taskMachine->updateStatus("TaskPreparation", TaskStepStatus::Running);
    auto taskPlan = taskMachine->taskPlan();
    auto& deviceWorkTask = taskPlan.deviceWorkTask;
    QString uuid = taskMachine->getMachineUUID();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 等待任务开始 */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto currentDateTime = GlobalData::currentDateTime();
    while (deviceWorkTask.m_preStartTime > currentDateTime)
    {
        if (isExit())
        {
            return;
        }
        QThread::msleep(800);
        currentDateTime = GlobalData::currentDateTime();
    };

    notifyInfoLog("任务准备流程开始执行");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 转换任务数据为当前流程所需要的数据 */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto taskTimeListResult = TaskRunCommonHelper::getDeviceWorkTaskInfo(taskPlan);
    if (!taskTimeListResult)
    {
        notifyErrorLog(taskTimeListResult.msg());
        VoiceAlarmPublish::publish("计划解析错误任务失败");
        notify("error");
        return;
    }
    m_taskTimeList = taskTimeListResult.value();

    notifyInfoLog(QString("任务开始，当前主跟任务ID:%1").arg(m_taskTimeList.masterSataTaskCode));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 下宏的消息数据 */
    m_taskTimeList.manualMessage = createManualMessage();
    m_taskTimeList.manualMessage.masterTaskCode = m_taskTimeList.masterSataTaskCode;
    m_taskTimeList.manualMessage.masterTaskBz = m_taskTimeList.masterSataTaskBz;
    auto allocResult = ControlFlowHandler::allocConfigMacro(m_taskTimeList.manualMessage, true);
    if (!allocResult)
    {
        notifyErrorLog(QString("资源查找错误:%1").arg(allocResult.msg()));
        VoiceAlarmPublish::publish("计划解析错误任务失败");
        notify("error");
        return;
    }
    TaskGlobalInfoSingleton::getInstance().setTaskTimeList(uuid, m_taskTimeList);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 计划检查 资源获取 设备检查 */
    if (!getTaskResourcesAndCheck())
    {
        notify("error");
        return;
    }

    VoiceAlarmPublish::publish("自动运行开始配置设备参数");
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* ACU处理 */
    configACU();
    if (isExit())
    {
        return;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 参数宏配置宏下发 */
    setParamMacro();
    if (isExit())
    {
        return;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 注释掉用上面内个 */
    // configACU();
    // if (isExit())
    // {
    //     return;
    // }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 准备阶段默认关闭存盘送数，处理站址装订 */
    // deviceInit();
    // if (isExit())
    // {
    //     return;
    // }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    notifyInfoLog(QString("任务准备阶段准备完成"));

    TaskGlobalInfoSingleton::getInstance().setTaskTimeList(uuid, m_taskTimeList);
    taskMachine->updateStatus("TaskPreparation", TaskStepStatus::Finish);
    notify("ZeroCorrection");
}

ManualMessage TaskPreparation::createManualMessage()
{
    /*
     * 20211012 wp??
     * 情况说明
     * 在4413时对基带的使用有两种
     * 一主一备 和 双主机
     * 但是在4426上因为两六要求测控基带需要一主一备
     * 以此为基准在4413上的一些特殊模式比如双点频STTC，STTC+Kakuo2，STTC+SKuo2等
     * 就不能按照之前的模式进行
     * 所以在4426上基带增加一个模式STTC+KaKuo2 以此来达到一主一备的目的
     * 因此基于一主一备在4426上要考虑的模式就至于一下几种情况
     * STTC
     * SKuo2
     * Kakuo2
     * KT
     * SKuo2+Kakuo2
     *
     * 其他的模式如果要存在也会先更改基带的模式所以暂时不考虑
     */
    ManualMessage msg;
    /*
     * 自动化运行因为ACU本身计算轨根要耗时
     * 所以需要提前下宏
     * 这处理一下
     */
    msg.acuParam = false;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    // S标准TTC
    STTC_ConfigAndParamSetting(msg);

    // S扩二
    Skuo2_ConfigAndParamSetting(msg);

    // Ka扩二
    KaKuo2_ConfigAndParamSetting(msg);

    // 扩跳
    SKT_ConfigAndParamSetting(msg);

    // X低速
    XDS_ConfigAndParamSetting(msg);

    // Ka低速
    KaDS_ConfigAndParamSetting(msg);

    // Ka高速
    KaGS_ConfigAndParamSetting(msg);

    return msg;
}
void TaskPreparation::setParamMacro()
{
    /* 提前处理一下 默认DTE是3年的计划  在当前这里要改一下 */
    QDateTime dteTraceStartDateTime;
    QDateTime dteTaskEndDateTime;
    auto dteTaskCalc = [&](SystemWorkMode mode) {
        if (!m_taskTimeList.m_dataMap.contains(mode))
        {
            return;
        }
        for (auto& item : m_taskTimeList.m_dataMap[mode])
        {
            if (item.isMaster)
            {
                auto temp1 = item.m_trace_start_time;
                auto temp2 = item.m_task_end_time;
                if (!dteTraceStartDateTime.isValid() || dteTraceStartDateTime > temp1)
                {
                    dteTraceStartDateTime = temp1;
                }
                if (!dteTaskEndDateTime.isValid() || dteTaskEndDateTime < temp2)
                {
                    dteTaskEndDateTime = temp2;
                }
            }
        }
    };

    dteTaskCalc(STTC);
    dteTaskCalc(Skuo2);
    dteTaskCalc(SKT);
    dteTaskCalc(KaKuo2);
    dteTaskCalc(KaDS);
    m_taskTimeList.manualMessage.ltDTETask = false;
    m_taskTimeList.manualMessage.dteTraceStartDateTime = dteTraceStartDateTime;
    m_taskTimeList.manualMessage.dteTaskEndDateTime = dteTaskEndDateTime;

    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &BaseEvent::notifyInfoLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &BaseEvent::notifyWarningLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &BaseEvent::notifyErrorLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalSpecificTipsMsg, this, &BaseEvent::notifySpecificTipsLog);

    controlFlowHandler.setPlanInfo(m_taskPlanData);
    controlFlowHandler.setRunningFlag(m_runningFlag);
    controlFlowHandler.handle(m_taskTimeList.manualMessage, true);
    m_taskTimeList.manualMessage = controlFlowHandler.getNewManualMsg();
    VoiceAlarmPublish::publish("参数宏执行成功");
}
void TaskPreparation::STTC_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = STTC;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    // STTC 在26上不考虑双点频
    auto taskTimeEntity = taskTimeEntityList.first();

    LinkLine linkLine;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = taskTimeEntity.isMaster ? 1 : -1;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = taskTimeEntity.m_taskCode;
    targetInfo.pointFreqNo = taskTimeEntity.m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::Skuo2_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = Skuo2;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    LinkLine linkLine;
    linkLine.workMode = workMode;
    for (auto& item : taskTimeEntityList)
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = workMode;
        targetInfo.taskCode = item.m_taskCode;
        targetInfo.pointFreqNo = item.m_dpNum;
        if (item.isMaster)
        {
            linkLine.masterTargetNo = index;
        }
        linkLine.targetMap[index++] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::KaKuo2_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = KaKuo2;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    LinkLine linkLine;
    linkLine.workMode = workMode;
    for (auto& item : taskTimeEntityList)
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = workMode;
        targetInfo.taskCode = item.m_taskCode;
        targetInfo.pointFreqNo = item.m_dpNum;
        if (item.isMaster)
        {
            linkLine.masterTargetNo = index;
        }
        linkLine.targetMap[index++] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::SKT_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = SKT;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }
    // SKT 在26上不考虑双点频
    auto taskTimeEntity = taskTimeEntityList.first();

    LinkLine linkLine;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = taskTimeEntity.isMaster ? 1 : -1;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = taskTimeEntity.m_taskCode;
    targetInfo.pointFreqNo = taskTimeEntity.m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::XDS_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = XDS;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    LinkLine linkLine;
    linkLine.workMode = workMode;
    for (auto& item : taskTimeEntityList)
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = workMode;
        targetInfo.taskCode = item.m_taskCode;
        targetInfo.pointFreqNo = item.m_dpNum;
        if (item.isMaster)
        {
            linkLine.masterTargetNo = index;
        }
        linkLine.targetMap[index++] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::KaDS_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = KaDS;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    LinkLine linkLine;
    linkLine.workMode = workMode;
    for (auto& item : taskTimeEntityList)
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = workMode;
        targetInfo.taskCode = item.m_taskCode;
        targetInfo.pointFreqNo = item.m_dpNum;
        if (item.isMaster)
        {
            linkLine.masterTargetNo = index;
        }
        linkLine.targetMap[index++] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}
void TaskPreparation::KaGS_ConfigAndParamSetting(ManualMessage& msg)
{
    auto index = 1;
    auto workMode = KaGS;
    if (!m_taskTimeList.m_dataMap.contains(workMode))
    {
        return;
    }
    auto& taskTimeEntityList = m_taskTimeList.m_dataMap.value(workMode);
    if (taskTimeEntityList.isEmpty())
    {
        return;
    }

    LinkLine linkLine;
    linkLine.workMode = workMode;
    for (auto& item : taskTimeEntityList)
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = workMode;
        targetInfo.taskCode = item.m_taskCode;
        targetInfo.pointFreqNo = item.m_dpNum;
        if (item.isMaster)
        {
            linkLine.masterTargetNo = index;
        }
        linkLine.targetMap[index++] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
    msg.appendLine(linkLine);
}

bool TaskPreparation::getTaskResourcesAndCheck()
{
    /* 1.检查任务参数结构是否有效 */
    if (m_taskTimeList.m_dataMap.isEmpty())
    {
        notifyErrorLog("计划解析错误任务失败");
        VoiceAlarmPublish::publish("计划解析错误任务失败");
        return false;
    }
    for (const auto& item : m_taskTimeList.m_dataMap)
    {
        if (item.isEmpty())
        {
            notifyErrorLog("计划解析错误任务失败");
            VoiceAlarmPublish::publish("计划解析错误任务失败");
            return false;
        }
    }

    /* 2.获取设备信息 */
    if (!getTaskResources())
    {
        return false;
    }
    /* 3. 设备检查*/
    return checkDeviceStatus();
}
bool TaskPreparation::getTaskResources()
{
    /*
     * 20211012 wp??
     * 情况说明
     * 在4413时对基带的使用有两种
     * 一主一备 和 双主机
     * 但是在4426上因为两六要求测控基带需要一主一备
     * 以此为基准在4413上的一些特殊模式比如双点频STTC，STTC+Kakuo2，STTC+SKuo2等
     * 就不能按照之前的模式进行
     * 所以在4426上基带增加一个模式STTC+KaKuo2 以此来达到一主一备的目的
     * 因此基于一主一备在4426上要考虑的模式就至于一下几种情况
     * STTC
     * SKuo2
     * Kakuo2
     * KT
     * SKuo2+Kakuo2
     *
     * 其他的模式如果要存在也会先更改基带的模式所以暂时不考虑
     */
    //计算点频数量
    auto sttcSize = m_taskTimeList.m_dataMap.value(STTC).size();
    auto skuo2Size = m_taskTimeList.m_dataMap.value(Skuo2).size();
    auto kakuo2Size = m_taskTimeList.m_dataMap.value(KaKuo2).size();
    auto sktSize = m_taskTimeList.m_dataMap.value(SKT).size();

    if (sttcSize >= 2)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S标准点频数量超过1个，系统基带数量不足支撑功能");
        return false;
    }
    if (sktSize >= 2)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S扩跳点频数量超过1个，系统基带数量不足支撑功能");
        return false;
    }
    if (skuo2Size > 4 || kakuo2Size > 4)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("扩频点频数量超过4个，系统基带数量不足支撑功能");
        return false;
    }

    if (sttcSize >= 1 && (skuo2Size >= 1 || kakuo2Size >= 1))
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S标准和S扩二不能同时执行任务，系统链路不足支撑功能");
        return false;
    }

    if (sktSize >= 1 && (skuo2Size >= 1 || kakuo2Size >= 1))
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S扩跳和S扩二不能同时执行任务，系统链路不足支撑功能");
        return false;
    }
    /* 获取主跟的参数宏 */
    if (!GlobalData::getParamMacroData(m_taskTimeList.manualMessage.masterTaskCode, m_taskTimeList.masterParamMacroData))
    {
        VoiceAlarmPublish::publish("主跟目标参数宏查找失败");
        notifyErrorLog(QString("主跟目标%1参数宏查找失败").arg(m_taskTimeList.manualMessage.masterTaskCode));
        return false;
    }

    return true;
}
bool TaskPreparation::checkDeviceStatus()
{
    //获取基带的在线状态和本控状态
    auto onlineACUResult = BaseHandler::getOnlineACU();
    if (!onlineACUResult)
    {
        notifyErrorLog(onlineACUResult.msg());
        VoiceAlarmPublish::publish("ACU离线");
        return false;
    }

    m_taskTimeList.onlineACU = onlineACUResult.value();

    if (!GlobalData::getDeviceSelfControl(m_taskTimeList.onlineACU))
    {
        notifyErrorLog(QString("当前主用ACU(%1机)处于分控，系统无法下发指令").arg(m_taskTimeList.onlineACU.toInt() == ACU_A ? "A" : "B"));
        VoiceAlarmPublish::publish("ACU分控");
    }

    /* 检查基带在线本分控情况 */
    auto checkFunc = [&](SystemWorkMode mode) {
        auto& configMacroData = m_taskTimeList.manualMessage.configMacroData;
        if (configMacroData.configMacroCmdModeMap.contains(mode))
        {
            QString devName;
            DeviceID deviceID;
            if (SystemWorkModeHelper::isDsDataTransmissionWorkMode(mode))
            {
                devName = "低速基带";
                configMacroData.getDSJDDeviceID(mode, deviceID);
            }
            else if (SystemWorkModeHelper::isGsDataTransmissionWorkMode(mode))
            {
                devName = "高速基带";
                configMacroData.getGSJDDeviceID(mode, deviceID);
            }
            else
            {
                return true;
            }
            if (!deviceID.isValid())
            {
                notifyErrorLog(QString("获取%1主机错误，系统无法下发指令").arg(devName));
                return false;
            }

            if (!GlobalData::getDeviceOnline(deviceID))
            {
                notifyErrorLog(QString("%1%2(主机)离线，系统无法下发指令").arg(devName, deviceID.extenID == 1 ? "A" : "B"));
                VoiceAlarmPublish::publish(QString("%1离线").arg(devName));
                return false;
            }
            if (!GlobalData::getDeviceSelfControl(deviceID))
            {
                notifyErrorLog(QString("%1%2(主机)处于分控，系统无法下发指令").arg(devName, deviceID.extenID == 1 ? "A" : "B"));
                VoiceAlarmPublish::publish(QString("%1分控").arg(devName));
                return false;
            }
        }
        return true;
    };

    checkFunc(XDS);
    checkFunc(KaDS);
    checkFunc(KaGS);

    // 测控基带不能像上面那样检查
    // 测控有4个设备ID 4001/4002/4003/4004
    // 再不同模式下ID不一样直接从资源重组获取的ID直接判断是不行的
    // 直接写死

    QSet<SystemWorkMode> workModeSet;
    for (auto iter = m_taskTimeList.m_dataMap.begin(); iter != m_taskTimeList.m_dataMap.end(); ++iter)
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
        return true;
    }

    auto ckCheckFunc = [&](const QList<DeviceID>& deviceMList, const QString& name) {
        int online = 0;
        int selfControl = 0;
        for (auto& item : deviceMList)
        {
            auto result = GlobalData::getDeviceOnline(item);
            online += static_cast<int>(result);
            if (!result)
            {
                continue;
            }

            result = GlobalData::getDeviceSelfControl(item);
            selfControl += static_cast<int>(result);
            if (!result)
            {
                return false;
            }
            break;
        }

        if (online <= 0)
        {
            notifyErrorLog(QString("%1离线，系统无法下发指令").arg(name));
            VoiceAlarmPublish::publish(QString("%1离线").arg(name));
            return false;
        }

        if (selfControl <= 0)
        {
            notifyErrorLog(QString("%1处于分控，系统无法下发指令").arg(name));
            VoiceAlarmPublish::publish(QString("%1分控").arg(name));
            return false;
        }
        return true;
    };

    QList<DeviceID> tempDeviceList;
    tempDeviceList << DeviceID(4, 0, 1);
    tempDeviceList << DeviceID(4, 0, 2);
    ckCheckFunc(tempDeviceList, "测控基带A");

    tempDeviceList.clear();
    tempDeviceList << DeviceID(4, 0, 3);
    tempDeviceList << DeviceID(4, 0, 4);
    ckCheckFunc(tempDeviceList, "测控基带B");

    // 处理跟踪模式
    // 当前下发的任务模式和设置的跟踪模式对应不上时需要处理一下
    auto trackingInfoResult = BaseHandler::checkTrackingMode(m_taskTimeList.manualMessage, m_taskTimeList.trackingMode);
    if (!trackingInfoResult)
    {
        auto msg = QString("计划解析错误:%1").arg(trackingInfoResult.msg());
        notifyErrorLog(msg);
        return false;
    }
    m_taskTimeList.trackingMode = trackingInfoResult.value();
    return true;
}

void TaskPreparation::configACU()
{
    if (m_taskTimeList.m_dataMap.contains(XDS) && m_taskTimeList.m_dataMap.size() == 1)
    {
        return;
    }

    auto acuDeviceID = m_taskTimeList.onlineACU;

    // 先下宏
    ACUParamMacroHandler acuParamMacroHandler;
    acuParamMacroHandler.setManualMessage(m_taskTimeList.manualMessage);
    acuParamMacroHandler.setRunningFlag(m_runningFlag);
    connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalSendToDevice, this, &TaskPreparation::signalSendToDevice);
    connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalInfoMsg, this, &TaskPreparation::notifyInfoLog);
    connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalWarningMsg, this, &TaskPreparation::notifyWarningLog);
    connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalErrorMsg, this, &TaskPreparation::notifyErrorLog);
    connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalSpecificTipsMsg, this, &TaskPreparation::notifySpecificTipsLog);

    acuParamMacroHandler.handle();

    QMap<QString, QVariant> acuTaskLoadParamMap;
    acuTaskLoadParamMap["RaskIdent"] = m_taskTimeList.masterSataTaskBz;                                          // 任务标识
    acuTaskLoadParamMap["TaskPrepDate"] = m_taskTimeList.m_task_ready_start_time.toString(DATE_DISPLAY_FORMAT);  // 任务准备开始日期
    acuTaskLoadParamMap["TaskPrepTime"] = m_taskTimeList.m_task_ready_start_time.toString(TIME_DISPLAY_FORMAT);  // 任务准备开始时间
    acuTaskLoadParamMap["TaskStartDate"] = m_taskTimeList.m_task_start_time.toString(DATE_DISPLAY_FORMAT);       // 任务开始日期
    acuTaskLoadParamMap["TaskStartTime"] = m_taskTimeList.m_task_start_time.toString(TIME_DISPLAY_FORMAT);       // 任务开始时间
    acuTaskLoadParamMap["TrackStartDate"] = m_taskTimeList.m_track_start_time.toString(DATE_DISPLAY_FORMAT);     // 跟踪开始日期
    acuTaskLoadParamMap["TrackStartTime"] = m_taskTimeList.m_track_start_time.toString(TIME_DISPLAY_FORMAT);     // 跟踪开始时间
    acuTaskLoadParamMap["TrackEndDate"] = m_taskTimeList.m_track_end_time.toString(DATE_DISPLAY_FORMAT);         // 跟踪结束日期
    acuTaskLoadParamMap["TrackEndTime"] = m_taskTimeList.m_track_end_time.toString(TIME_DISPLAY_FORMAT);         // 跟踪结束时间
    acuTaskLoadParamMap["TaskEndDate"] = m_taskTimeList.m_task_end_time.toString(DATE_DISPLAY_FORMAT);           //任务结束日期
    acuTaskLoadParamMap["TaskEndTime"] = m_taskTimeList.m_task_end_time.toString(TIME_DISPLAY_FORMAT);           // 任务结束时间

    if (m_taskTimeList.m_dt_starttime.isValid())
    {
        acuTaskLoadParamMap["DTStartDate"] = m_taskTimeList.m_dt_starttime.toString(DATE_DISPLAY_FORMAT);  // 数传结束时间
        acuTaskLoadParamMap["DTStartTime"] = m_taskTimeList.m_dt_starttime.toString(TIME_DISPLAY_FORMAT);  // 数传结束时间
        acuTaskLoadParamMap["DTEndDate"] = m_taskTimeList.m_dt_endtime.toString(DATE_DISPLAY_FORMAT);      // 数传结束时间
        acuTaskLoadParamMap["DTEndTime"] = m_taskTimeList.m_dt_endtime.toString(TIME_DISPLAY_FORMAT);      // 数传结束时间
    }
    else
    {
        auto currentDateTime = GlobalData::currentDateTime();
        acuTaskLoadParamMap["DTStartDate"] = currentDateTime.toString(DATE_DISPLAY_FORMAT);  // 数传结束时间
        acuTaskLoadParamMap["DTStartTime"] = currentDateTime.toString(TIME_DISPLAY_FORMAT);  // 数传结束时间
        acuTaskLoadParamMap["DTEndDate"] = currentDateTime.toString(DATE_DISPLAY_FORMAT);    // 数传结束时间
        acuTaskLoadParamMap["DTEndTime"] = currentDateTime.toString(TIME_DISPLAY_FORMAT);    // 数传结束时间
    }

    m_singleCommandHelper.packSingleCommand("Step_ACU_JHXF", m_packCommand, acuDeviceID, acuTaskLoadParamMap);
    waitExecSuccess(m_packCommand);

//    // 参数宏下发完成后运行方式（自动）下发ACU   ACU运行方式设置为自动运行
//    m_singleCommandHelper.packSingleCommand("Step_ACU_OperatMode_AutoRun", m_packCommand, acuDeviceID);
//    waitExecSuccess(m_packCommand);

}

// void TaskPreparation::deviceInit()
//{
//    /* 20211120 先注释掉 因为站上要求下宏默认打开送数 那这里关闭就没什么必要了 */
//    ResourceReleaseDispatcher resourceReleaseDispatcher;
//    connect(&resourceReleaseDispatcher, &ResourceReleaseDispatcher::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
//    connect(&resourceReleaseDispatcher, &ResourceReleaseDispatcher::signalInfoMsg, this, &BaseEvent::notifyInfoLog);
//    connect(&resourceReleaseDispatcher, &ResourceReleaseDispatcher::signalWarningMsg, this, &BaseEvent::notifyWarningLog);
//    connect(&resourceReleaseDispatcher, &ResourceReleaseDispatcher::signalErrorMsg, this, &BaseEvent::notifyErrorLog);
//    connect(&resourceReleaseDispatcher, &ResourceReleaseDispatcher::signalSpecificTipsMsg, this, &BaseEvent::notifySpecificTipsLog);

//    auto bak = m_taskTimeList.manualMessage;
//    bak.resourceReleaseDeleteDTETask = false;
//    bak.manualType = ManualType::ResourceRelease;
//    resourceReleaseDispatcher.handle(bak);
//}
