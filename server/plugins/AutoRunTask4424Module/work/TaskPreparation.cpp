#include "TaskPreparation.h"

#include "AutorunPolicyMessageDefine.h"
#include "BaseHandler.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroDispatcher.h"
#include "ControlFlowHandler.h"
#include "GlobalData.h"
#include "LinkConfigHelper.h"
#include "MessagePublish.h"
#include "ParamMacroDispatcher.h"
#include "PlanRunMessageDefine.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"

#include "MessagePublish.h"
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

    const DeviceWorkTask deviceWorkTask = taskMachine->deviceWorkTask();
    QString uuid = taskMachine->getMachineUUID();

    // 等待任务准备执行
    //    auto currentDateTime = GlobalData::currentDateTime();
    //    while (deviceWorkTask.m_preStartTime > currentDateTime)
    //    {
    //        QThread::msleep(800);
    //        currentDateTime = GlobalData::currentDateTime();
    //    }

    notifyInfoLog("任务准备流程开始执行");

    // 转换任务数据为当前流程所需要的数据
    auto taskTimeListResult = TaskRunCommonHelper::getDeviceWorkTaskInfo(deviceWorkTask);
    if (!taskTimeListResult)
    {
        notifyErrorLog(taskTimeListResult.msg());
        VoiceAlarmPublish::publish("计划解析错误任务失败");
        notify("error");
        return;
    }
    m_taskTimeList = taskTimeListResult.value();
    notifyInfoLog(QString("任务开始，当前任务ID:%1").arg(m_taskTimeList.masterTaskCode));

    /* 获取下宏的消息数据 */
    m_taskTimeList.manualMessage = createManualMessage();
    auto allocResult = ControlFlowHandler::allocConfigMacro(m_taskTimeList.manualMessage, m_taskTimeList.masterTaskCode);
    if (!allocResult)
    {
        notifyErrorLog(QString("资源查找错误:%1").arg(allocResult.msg()));
        VoiceAlarmPublish::publish("计划解析错误任务失败");
        notify("error");
        return;
    }

    /* 计划检查 资源获取 设备检查 */
    if (!getTaskResourcesAndCheck())
    {
        notify("error");
        //        return;  暂时注释
    }

    /* 参数宏配置宏下发 */
    setParamMacro();

    /* DTE处理 */
    configDTE();

    /* ACU处理 */
    configACU();

    /* 角误差信号给ACU */
    configGZDeviceJWC();

    /* 存储转发处理 */
    //存储转发  Ka高速
    configCCZFKaHS();

    /* 准备阶段默认关闭存盘送数，处理站址装订 */
    deviceInit();

    notifyInfoLog(QString("任务准备阶段准备完成"));

    TaskGlobalInfoSingleton::getInstance().setTaskTimeList(uuid, m_taskTimeList);
    notify("ZeroCorrection");
}

ManualMessage TaskPreparation::createManualMessage()
{
    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;

    // S标准TTC
    if (m_taskTimeList.m_dataMap.contains(STTC))
    {
        LinkLine linkLine;
        STTC_ConfigAndParamSetting(linkLine);
        msg.appendLine(linkLine);
    }
    // S扩二
    if (m_taskTimeList.m_dataMap.contains(Skuo2))
    {
        LinkLine linkLine;
        Skuo2_ConfigAndParamSetting(linkLine);
        msg.appendLine(linkLine);
    }
    // Ka扩二
    if (m_taskTimeList.m_dataMap.contains(KaKuo2))
    {
        LinkLine linkLine;
        KaKuo2_ConfigAndParamSetting(linkLine);
        msg.appendLine(linkLine);
    }
    // 扩跳
    if (m_taskTimeList.m_dataMap.contains(SKT))
    {
        LinkLine linkLine;
        SKT_ConfigAndParamSetting(linkLine);
        msg.appendLine(linkLine);
    }
    // Ka高速
    if (m_taskTimeList.m_dataMap.contains(KaGS))
    {
        LinkLine linkLine;
        KaGS_ConfigAndParamSetting(linkLine);
        msg.appendLine(linkLine);
    }
    return msg;
}

void TaskPreparation::STTC_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = STTC;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::Skuo2_ConfigAndParamSetting(LinkLine& linkLine)
{
    int index = 1;
    auto workMode = Skuo2;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    for (auto item : m_taskTimeList.m_dataMap[workMode])
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
        linkLine.targetMap[index] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::KaKuo2_ConfigAndParamSetting(LinkLine& linkLine)
{
    int index = 1;
    auto workMode = KaKuo2;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    for (auto item : m_taskTimeList.m_dataMap[workMode])
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
        linkLine.targetMap[index] = targetInfo;
    }

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::SYTHSMJ_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = SYTHSMJ;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::SYTHWX_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = SYTHWX;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::SYTHGML_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = SYTHGML;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::SYTHSMJK2GZB_ConfigAndParamSetting(LinkLine& linkLine) {}

void TaskPreparation::SYTHSMJK2BGZB_ConfigAndParamSetting(LinkLine& linkLine) {}

void TaskPreparation::SYTHWXSK2_ConfigAndParamSetting(LinkLine& linkLine) {}

void TaskPreparation::SYTHGMLSK2_ConfigAndParamSetting(LinkLine& linkLine) {}

void TaskPreparation::SKT_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = SKT;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::XDS_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = XDS;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::KaDS_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = KaDS;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::XGS_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = XGS;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::KaGS_ConfigAndParamSetting(LinkLine& linkLine)
{
    auto index = 1;
    auto workMode = KaGS;
    linkLine.workMode = workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = workMode;
    targetInfo.taskCode = m_taskTimeList.m_dataMap[workMode].first().m_taskCode;
    targetInfo.pointFreqNo = m_taskTimeList.m_dataMap[workMode].first().m_dpNum;
    linkLine.targetMap[index] = targetInfo;

    linkLine.linkType = LinkType::RWFS;
}

void TaskPreparation::setParamMacro()
{
    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &BaseEvent::notifyInfoLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &BaseEvent::notifyWarningLog);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &BaseEvent::notifyErrorLog);

    controlFlowHandler.handle(m_taskTimeList.manualMessage, m_taskTimeList.masterTaskCode);
    m_taskTimeList.manualMessage = controlFlowHandler.getNewManualMsg();
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

    /* 2.获取站测控UAC地址 */
    auto staticUACResult = TaskRunCommonHelper::getStationCKUAC();
    if (!staticUACResult)
    {
        notifyErrorLog(staticUACResult.msg());
        return false;
    }
    m_taskTimeList.txUACAddr = staticUACResult.value();

    /* 3.获取设备信息 */
    if (!getTaskResources())
    {
        return false;
    }
    /* 4. 设备检查*/
    return checkDeviceStatus();
}

bool TaskPreparation::checkDeviceStatus()
{
    //获取基带的在线状态和本控状态
    auto onlineACUResult = getOnlineACU();
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
    auto checkFunc = [&](SystemWorkMode mode, const QString& devName, const QMap<int, DeviceID>& deviceMap) {
        if (m_taskTimeList.m_dataMap.contains(mode) && deviceMap.contains(mode))
        {
            auto ckDev = deviceMap.value(mode);
            if (!GlobalData::getDeviceOnline(ckDev))
            {
                notifyErrorLog(QString("%1%2(主机)离线，系统无法下发指令").arg(devName, ckDev.extenID == 1 ? "A" : "B"));
                VoiceAlarmPublish::publish(QString("%1离线").arg(devName));
                return false;
            }
            if (!GlobalData::getDeviceSelfControl(ckDev))
            {
                notifyErrorLog(QString("%1%2(主机)处于分控，系统无法下发指令").arg(devName, ckDev.extenID == 1 ? "A" : "B"));
                VoiceAlarmPublish::publish(QString("%1分控").arg(devName));
                return false;
            }
        }
        return true;
    };

    // 这里一体化+扩频的有问题，目前先统一处理
    checkFunc(STTC, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(Skuo2, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHSMJ, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHWX, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHGML, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHSMJK2GZB, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHSMJK2BGZB, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHWXSK2, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(SYTHGMLSK2, "测控基带", m_taskTimeList.m_ckDeviceMap);
    checkFunc(KaGS, "高速基带", m_taskTimeList.m_gsDeviceMap);

    return true;
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
    //如果STTC/S扩2 + ka扩二的设备数量超过3 就代表资源不足，系统无法执行
    int kak2size = 0;
    const auto& modeExist = m_taskTimeList.modeExist;
    if (modeExist.kakuo2Mode)
    {
        kak2size = 1;
    }
    if (modeExist.sttcMode && m_taskTimeList.m_dataMap[STTC].size() > 2)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S标准点频数量超过2个，系统基带数量不足支撑功能");
        return false;
    }
    /* 上面已经检查过了 最少都是1个 */
    if (modeExist.sttcMode &&   //
        modeExist.skuo2Mode &&  //
        m_taskTimeList.m_dataMap[STTC].size() > 1)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段S标准和S扩二 不能同时执行任务，系统链路不足支撑功能");
        return false;
    }
    if (modeExist.sttcMode &&   //
        modeExist.skuo2Mode &&  //
        m_taskTimeList.m_dataMap[STTC].size() + m_taskTimeList.m_dataMap[Skuo2].size() + kak2size > 2)
    {
        VoiceAlarmPublish::publish("任务测控模式过多系统不支持");
        notifyErrorLog("S频段和ka频段测控点频数量超过3个，系统基带数量不足支撑功能");
        return false;
    }
    DeviceID sttcDeviceID;
    DeviceID skuo2DeviceID;
    DeviceID kakuo2DeviceID;
    DeviceID sktDeviceID;
    DeviceID xdsDeviceID;
    DeviceID kadsDeviceID;
    DeviceID kagsDeviceID;

    using GetDeviceFunc = bool (*)(const ConfigMacroData&, SystemWorkMode, DeviceID&);
    auto deviceFoundFunc = [&](SystemWorkMode mode, DeviceID& deviceID, GetDeviceFunc getDevice) {
        if (!m_taskTimeList.m_dataMap.contains(mode))
        {
            return true;
        }
        return getDevice(m_taskTimeList.manualMessage.configMacroData, mode, deviceID);
    };

    std::array<std::tuple<SystemWorkMode, QString, DeviceID*, GetDeviceFunc>, 7> deviceFoundArray = {
        std::make_tuple(STTC, "获取主用测控基带失败", &sttcDeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getCKJDDeviceID(mode, deviceID);
                        }),

        std::make_tuple(Skuo2, "获取主用测控基带失败", &skuo2DeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getCKJDDeviceID(mode, deviceID);
                        }),
        std::make_tuple(KaKuo2, "获取主用测控基带失败", &kakuo2DeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getCKJDDeviceID(mode, deviceID);
                        }),

        std::make_tuple(SKT, "获取主用测控基带失败", &sktDeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getCKJDDeviceID(mode, deviceID);
                        }),
        std::make_tuple(XDS, "获取主用低速基带失败", &xdsDeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getDSJDDeviceID(mode, deviceID);
                        }),

        std::make_tuple(KaDS, "获取主用低速基带失败", &kadsDeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getDSJDDeviceID(mode, deviceID);
                        }),
        std::make_tuple(KaGS, "获取主用高速基带失败", &kagsDeviceID,
                        [](const ConfigMacroData& configMacroData, SystemWorkMode mode, DeviceID& deviceID) {
                            return configMacroData.getGSJDDeviceID(mode, deviceID);
                        }),

    };

    for (auto& item : deviceFoundArray)
    {
        if (!deviceFoundFunc(std::get<0>(item), *std::get<2>(item), std::get<3>(item)))
        {
            VoiceAlarmPublish::publish("任务执行时所需设备查找失败");
            notifyErrorLog(std::get<1>(item));
            return false;
        }
    }
    // 更新基带重组策略
    if (modeExist.sttcMode)
    {
        m_taskTimeList.m_ckDeviceMap[STTC] = sttcDeviceID;
    }
    if (modeExist.skuo2Mode)
    {
        m_taskTimeList.m_ckDeviceMap[Skuo2] = skuo2DeviceID;
    }
    if (modeExist.kakuo2Mode)
    {
        m_taskTimeList.m_ckDeviceMap[KaKuo2] = kakuo2DeviceID;
    }
    if (modeExist.sktMode)
    {
        m_taskTimeList.m_ckDeviceMap[SKT] = sktDeviceID;
    }

    /*
     * 20210704
     * 高速只能使用通道一和通道二不和低速打架单独走就行
     *
     * X单点频 或 Ka低速单点频 或 X单点频+Ka低速单点频 使用资源重组的设备
     * X双点频+Ka低速单点频  或者 X单点频+Ka低速双点频 或 X双点频+Ka低速双点频 使用两个存转设备
     *
     * 当X低速与Ka低速同时存在且任意一个有双点频
     * 那么存转就一定需要两台，就需要重新分配X和Ka低速的设备
     *
     */
    DeviceID xdsCCZFDeviceID(8, 1, 1);
    DeviceID kadsCCZFDeviceID(8, 1, 1);
    DeviceID kagsCCZFDeviceID(8, 1, 1);
    if ((modeExist.kadsMode && modeExist.xdsMode) &&  //
        (m_taskTimeList.m_dataMap[KaDS].size() >= 2 || m_taskTimeList.m_dataMap[XDS].size() >= 2))
    {
        /* 检查两种模式的存转是否冲突,冲突就重新分配 */
        if (kadsCCZFDeviceID == xdsCCZFDeviceID)
        {
            if (kadsCCZFDeviceID.extenID == 1)
            {
                xdsCCZFDeviceID.extenID = 2;
            }
            else
            {
                xdsCCZFDeviceID.extenID = 1;
            }
            notifyWarningLog(QString("Ka低速与X低速冲突,重新分配后Ka低速使用存转%1 X低速使用存转%2")
                                 .arg((kadsCCZFDeviceID.extenID == 1 ? "A" : "B"), (xdsCCZFDeviceID.extenID == 1 ? "A" : "B")));
        }
    }

    if (modeExist.xdsMode)
    {
        m_taskTimeList.m_dsDeviceMap[XDS] = xdsDeviceID;
        m_taskTimeList.m_satDeviceMap[XDS] = xdsCCZFDeviceID;
        m_taskTimeList.m_xdsChannel = 4;
    }
    if (modeExist.kadsMode)
    {
        m_taskTimeList.m_dsDeviceMap[KaDS] = kadsDeviceID;
        m_taskTimeList.m_satDeviceMap[KaDS] = kadsCCZFDeviceID;
        m_taskTimeList.m_kadsChannel = 3;
    }
    if (modeExist.kagsMode)
    {
        m_taskTimeList.m_gsDeviceMap[KaGS] = kagsDeviceID;
        m_taskTimeList.m_satDeviceMap[KaGS] = kagsCCZFDeviceID;
    }

    // 跟踪基带 功放
    for (auto iter = m_taskTimeList.m_dataMap.begin(); iter != m_taskTimeList.m_dataMap.end(); ++iter)
    {
        auto workMode = SystemWorkMode(iter.key());
        DeviceID deviceID;
        if (m_taskTimeList.manualMessage.configMacroData.getGZJDDeviceID(workMode, deviceID) && deviceID.isValid())
        {
            m_taskTimeList.m_gzDeviceMap[workMode] = deviceID;
            deviceID.resz();
        }

        auto band = SystemWorkModeHelper::systemWorkModeToSystemBand(workMode);
        if (band == SystemBandMode::SBand)
        {
            m_taskTimeList.manualMessage.configMacroData.getSGGFDeviceID(workMode, deviceID);
            if (!deviceID.isValid())
            {
                deviceID.sysID = 2;
                deviceID.devID = 0;
                deviceID.extenID = 1;
            }
            m_taskTimeList.m_hpaDeviceMap[workMode] = deviceID;
        }
        else if (workMode == KaKuo2)
        {
            m_taskTimeList.manualMessage.configMacroData.getKaGGFDeviceID(workMode, deviceID);
            if (!deviceID.isValid())
            {
                deviceID.sysID = 2;
                deviceID.devID = 0;
                deviceID.extenID = 3;
            }
            m_taskTimeList.m_hpaDeviceMap[workMode] = deviceID;
        }
        else if (workMode == KaDS)
        {
            m_taskTimeList.manualMessage.configMacroData.getKaDtGGFDeviceID(workMode, deviceID);
            if (!deviceID.isValid())
            {
                deviceID.sysID = 2;
                deviceID.devID = 0;
                deviceID.extenID = 5;
            }
            m_taskTimeList.m_hpaDeviceMap[workMode] = deviceID;
        }
    }
    //    /* 获取主跟的参数宏 */
    //    if (!GlobalData::getParamMacroData(m_taskTimeList.masterTaskCode, m_taskTimeList.masterParamMacroData))
    //    {
    //        VoiceAlarmPublish::publish("主跟目标参数宏查找失败");
    //        notifyErrorLog(QString("主跟目标%1参数宏查找失败").arg(m_taskTimeList.masterTaskCode));
    //        return false;
    //    }

    //    /* 自动化运行策略数据获取 */
    //    AutorunPolicyData autorunPolicyData;
    //    GlobalData::getAutorunPolicyData(autorunPolicyData);
    //    m_taskTimeList.commonTaskConfig.sttcCorrectValue = autorunPolicyData.policy.value("STTCXZ", 35).toDouble();
    //    m_taskTimeList.commonTaskConfig.skuo2CorrectValue = autorunPolicyData.policy.value("SKUO2XZ", 35).toDouble();
    //    m_taskTimeList.commonTaskConfig.kakuo2CorrectValue = autorunPolicyData.policy.value("KAKUO2XZ", 35).toDouble();
    //    m_taskTimeList.commonTaskConfig.kakuo2CorrectValue = autorunPolicyData.policy.value("KTXZ", 35).toDouble();

    //    m_taskTimeList.commonTaskConfig.sCrossLowLimit = autorunPolicyData.policy.value("S_CrossLowLimit", 5).toInt();
    //    m_taskTimeList.commonTaskConfig.sStandSensit = autorunPolicyData.policy.value("S_StandSensit", 500).toInt();
    //    m_taskTimeList.commonTaskConfig.sDirectToler = autorunPolicyData.policy.value("S_DirectToler", 50).toInt();

    //    auto value = static_cast<int>(PhaseCalibrationFollowCheck::NotCheck);
    //    m_taskTimeList.commonTaskConfig.sFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("S_FollowCheck", value).toInt());

    //    m_taskTimeList.commonTaskConfig.kaCrossLowLimit = autorunPolicyData.policy.value("Ka_CrossLowLimit", 5).toInt();
    //    m_taskTimeList.commonTaskConfig.kaStandSensit = autorunPolicyData.policy.value("Ka_StandSensit", 4800).toInt();
    //    m_taskTimeList.commonTaskConfig.kaDirectToler = autorunPolicyData.policy.value("Ka_DirectToler", 400).toInt();
    //    m_taskTimeList.commonTaskConfig.kaFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("Ka_FollowCheck",
    //    value).toInt());

    //    m_taskTimeList.commonTaskConfig.kadtCrossLowLimit = autorunPolicyData.policy.value("KADT_CrossLowLimit", 5).toInt();
    //    m_taskTimeList.commonTaskConfig.kadtStandSensit = autorunPolicyData.policy.value("KADT_StandSensit", 4800).toInt();
    //    m_taskTimeList.commonTaskConfig.kadtDirectToler = autorunPolicyData.policy.value("KADT_DirectToler", 400).toInt();
    //    m_taskTimeList.commonTaskConfig.kadtFollowCheck = PhaseCalibrationFollowCheck(autorunPolicyData.policy.value("KADT_FollowCheck",
    //    value).toInt());

    //    value = static_cast<int>(PhaseCalibrationCorrMeth::Satellite);
    //    m_taskTimeList.commonTaskConfig.phaseCorrMeth = PhaseCalibrationCorrMeth(autorunPolicyData.policy.value("PCaM", value).toInt());

    return true;
}

// void TaskPreparation::checkDeviceStatus(TaskTimeList m_taskTimeList)
//{
//    notifyInfoLog("任务中不检查当前任务状态");
//    return;

//    DeviceID deviceACUMaster;  //主用的ACU
//    auto onlineACUResult = getOnlineACU();
//    if (!onlineACUResult)
//    {
//        notifyErrorLog(onlineACUResult.msg());
//        VoiceAlarmPublish::publish("ACU离线");
//        notify("error");
//        return;
//    }
//    deviceACUMaster = onlineACUResult.value();

//    if (!GlobalData::getDeviceSelfControl(deviceACUMaster))
//    {
//        notifyErrorLog(QString("当前主用ACU(%1机)处于分控，系统无法下发指令").arg(deviceACUMaster == ACU_A ? "A" : "B"));
//        VoiceAlarmPublish::publish("ACU分控");
//    }

//    if (m_taskTimeList.m_dataMap.contains(STTC) && m_taskTimeList.m_ckDeviceMap.contains(STTC))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[STTC]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[STTC].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[STTC]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[STTC].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(STTC) && m_taskTimeList.m_ckDeviceMap.contains(STTC))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[STTC]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[STTC].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[STTC]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[STTC].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(Skuo2) && m_taskTimeList.m_ckDeviceMap.contains(Skuo2))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[Skuo2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[Skuo2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[Skuo2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[Skuo2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(KaKuo2) && m_taskTimeList.m_ckDeviceMap.contains(KaKuo2))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[KaKuo2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[KaKuo2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[KaKuo2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[KaKuo2].extenID == 1 ? "A" :
//            "B")); VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHSMJ) && m_taskTimeList.m_ckDeviceMap.contains(SYTHSMJ))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHSMJ]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJ].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHSMJ]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJ].extenID == 1 ? "A" :
//            "B")); VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHWX) && m_taskTimeList.m_ckDeviceMap.contains(SYTHWX))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHWX]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHWX].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHWX]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHWX].extenID == 1 ? "A" :
//            "B")); VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHGML) && m_taskTimeList.m_ckDeviceMap.contains(SYTHGML))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHGML]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHGML].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHGML]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHGML].extenID == 1 ? "A" :
//            "B")); VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHSMJK2GZB) && m_taskTimeList.m_ckDeviceMap.contains(SYTHSMJK2GZB))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2GZB]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2GZB].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2GZB]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2GZB].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHSMJK2BGZB) && m_taskTimeList.m_ckDeviceMap.contains(SYTHSMJK2BGZB))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2BGZB]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2BGZB].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2BGZB]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHSMJK2BGZB].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHWXSK2) && m_taskTimeList.m_ckDeviceMap.contains(SYTHWXSK2))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHWXSK2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHWXSK2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHWXSK2]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHWXSK2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SYTHGMLSK2) && m_taskTimeList.m_ckDeviceMap.contains(SYTHGMLSK2))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SYTHGMLSK2]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHGMLSK2].extenID == 1 ? "A" :
//            "B")); VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SYTHGMLSK2]))
//        {
//            notifyErrorLog(
//                QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SYTHGMLSK2].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    if (m_taskTimeList.m_dataMap.contains(SKT) && m_taskTimeList.m_ckDeviceMap.contains(SKT))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_ckDeviceMap[SKT]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SKT].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_ckDeviceMap[SKT]))
//        {
//            notifyErrorLog(QString("测控基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_ckDeviceMap[SKT].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }
//    /*X低速*/
//    if (m_taskTimeList.m_dataMap.contains(XDS) && m_taskTimeList.m_dsDeviceMap.contains(XDS))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_dsDeviceMap[XDS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_dsDeviceMap[XDS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_dsDeviceMap[XDS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_dsDeviceMap[XDS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    /*Ka低速*/
//    if (m_taskTimeList.m_dataMap.contains(KaDS) && m_taskTimeList.m_ckDeviceMap.contains(KaDS))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_dsDeviceMap[KaDS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_dsDeviceMap[KaDS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_dsDeviceMap[KaDS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_dsDeviceMap[KaDS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    /*X高速*/
//    if (m_taskTimeList.m_dataMap.contains(XGS) && m_taskTimeList.m_gsDeviceMap.contains(XGS))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_gsDeviceMap[XGS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_gsDeviceMap[XGS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_gsDeviceMap[XGS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_gsDeviceMap[XGS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }

//    /*Ka高速*/
//    if (m_taskTimeList.m_dataMap.contains(KaGS) && m_taskTimeList.m_gsDeviceMap.contains(KaGS))
//    {
//        if (GlobalData::getDeviceOnline(m_taskTimeList.m_gsDeviceMap[KaGS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)离线，系统无法下发指令").arg(m_taskTimeList.m_gsDeviceMap[KaGS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带离线");
//        }
//        if (!GlobalData::getDeviceSelfControl(m_taskTimeList.m_gsDeviceMap[KaGS]))
//        {
//            notifyErrorLog(QString("低速基带%1(主机)处于分控，系统无法下发指令").arg(m_taskTimeList.m_gsDeviceMap[KaGS].extenID == 1 ? "A" : "B"));
//            VoiceAlarmPublish::publish("测控基带分控");
//        }
//    }
//}

void TaskPreparation::configDTE()
{
    if (!dteOnlineCheck())
    {
        notifyErrorLog("未能检测到DTE，配置文件错误或者设备离线");
        VoiceAlarmPublish::publish("DTE状态获取失败");
        //        return;  没有设备，暂时注释，测试使用
    }

    // 发送DTE本控切换命令
    m_singleCommandHelper.packSingleCommand("Step_DTE_CTRLMODE", m_packCommand);
    waitExecSuccess(m_packCommand);

    /* 20211021 wp
     * 当有STTC Skuo2 基带编号为 1 2
     * 当有Ka扩2时 基带编号为3 4
     * 当有Ka低速 时会使用5 6
     * 当测控同时有Skuo2和Ka扩2时 测控会用4台基带
     * 其他单模式测控只占用2台基带
     * 低速只占用2台基带
     */
    bool dte_ycjd_s = false;
    bool dte_ycjd_ka = false;
    bool dte_dsjd = false;
    QDateTime dteTraceStartDateTime;
    QDateTime dteTaskEndDateTime;

    auto dteTaskCalc = [&](SystemWorkMode mode, bool& jdEnable) {
        if (!m_taskTimeList.m_dataMap.contains(mode))
        {
            return;
        }
        jdEnable = true;
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

    dteTaskCalc(STTC, dte_ycjd_s);
    dteTaskCalc(Skuo2, dte_ycjd_s);
    dteTaskCalc(SKT, dte_ycjd_s);
    dteTaskCalc(KaKuo2, dte_ycjd_ka);
    dteTaskCalc(XDS, dte_dsjd);
    dteTaskCalc(KaDS, dte_dsjd);

    if (!((dte_ycjd_s || dte_ycjd_ka) || dte_dsjd))
    {
        notifyErrorLog("计算当前基带使用数量失败,配置DTE任务错误");
        VoiceAlarmPublish::publish("DTE计划配置错误");
        notify("error");
        return;
    }

    /* 有任何一个时间无效 但是有当前的频段那就是数据出错了 */
    if (!dteTraceStartDateTime.isValid() || !dteTaskEndDateTime.isValid())
    {
        notifyErrorLog("当前计划时间错误,任务失败");
        VoiceAlarmPublish::publish("任务时间错误");
        notify("error");
        return;
    }

    QMap<QString, QVariant> dteTaskLoadParamMap;
    dteTaskLoadParamMap["MID"] = m_taskTimeList.masterTaskBz;                                            // 任务标识
    dteTaskLoadParamMap["UAC"] = m_taskTimeList.txUACAddr;                                               // 天线标识
    dteTaskLoadParamMap["TrackStartDate"] = dteTraceStartDateTime.date().toString(DATE_DISPLAY_FORMAT);  // 跟踪开始日期
    dteTaskLoadParamMap["TrackStartTme"] = dteTraceStartDateTime.time().toString(TIME_DISPLAY_FORMAT);   // 跟踪开始时间
    dteTaskLoadParamMap["MissionStopDate"] = dteTaskEndDateTime.date().toString(DATE_DISPLAY_FORMAT);    // 任务结束日期
    dteTaskLoadParamMap["MissionSStopTime"] = dteTaskEndDateTime.time().toString(TIME_DISPLAY_FORMAT);   // 任务结束时间
    /* DTE的任务是动态参数 基带的数量是可变的*/
    QList<int> bbeList;
    int bbeNum = 0;
    if (dte_ycjd_s && dte_ycjd_ka)
    {
        bbeNum += 4;
        bbeList << 1;
        bbeList << 2;
        bbeList << 3;
        bbeList << 4;
    }
    else if (dte_ycjd_s)
    {
        bbeNum += 2;
        bbeList << 1;
        bbeList << 2;
    }
    else if (dte_ycjd_ka)
    {
        bbeNum += 2;
        bbeList << 3;
        bbeList << 4;
    }

    if (dte_dsjd)
    {
        bbeNum += 2;
        bbeList << 5;
        bbeList << 6;
    }

    dteTaskLoadParamMap["BbeNum"] = bbeNum;
    // DTE的基带参数
    QMap<int, QList<QPair<QString, QVariant>>> dteJDNumMap;
    int index = 0;
    QList<QPair<QString, QVariant>> tempJDNumList;
    for (auto& item : bbeList)
    {
        tempJDNumList << qMakePair(QString("Bbe1Index%1").arg(index++), QVariant(item));
    }
    dteJDNumMap[1] = tempJDNumList;

    // 发送DTE_A任务加载命令
    m_singleCommandHelper.packSingleCommand("Step_DTE_LOADTASK", m_packCommand, dteJDNumMap, {}, dteTaskLoadParamMap);
    waitExecSuccess(m_packCommand);

    //根据任务代号获取任务标识，天线标识， 向DTE下发瞬根申请命令
    QMap<QString, QVariant> dteSunGenParamMap;
    dteSunGenParamMap["MID"] = m_taskTimeList.masterTaskBz;  // 任务标识
    dteSunGenParamMap["UAC"] = m_taskTimeList.txUACAddr;     // UAC

    // 发送DTE_A瞬根申请命令
    m_singleCommandHelper.packSingleCommand("Step_DTE_APPLY_OrbitalRoot", m_packCommand, dteSunGenParamMap);
    waitExecSuccess(m_packCommand);
}

void TaskPreparation::configACU()
{
    auto acuDeviceID = m_taskTimeList.onlineACU;

    for (auto& taskTimeEntity : m_taskTimeList.m_dataMap)
    {
        for (auto& item : taskTimeEntity)
        {
            QMap<QString, QVariant> acuTaskLoadParamMap;
            acuTaskLoadParamMap["RaskIdent"] = item.m_taskBz;                                                  // 任务标识
            acuTaskLoadParamMap["TaskPrepDate"] = item.m_task_ready_start_time.toString(DATE_DISPLAY_FORMAT);  // 任务准备开始日期
            acuTaskLoadParamMap["TaskPrepTime"] = item.m_task_ready_start_time.toString(TIME_DISPLAY_FORMAT);  // 任务准备开始时间
            acuTaskLoadParamMap["TaskStartDate"] = item.m_task_start_time.toString(DATE_DISPLAY_FORMAT);       // 任务开始日期
            acuTaskLoadParamMap["TaskStartTime"] = item.m_task_start_time.toString(TIME_DISPLAY_FORMAT);       // 任务开始时间
            acuTaskLoadParamMap["TrackStartDate"] = item.m_trace_start_time.toString(DATE_DISPLAY_FORMAT);     // 跟踪开始日期
            acuTaskLoadParamMap["TrackStartTime"] = item.m_trace_start_time.toString(TIME_DISPLAY_FORMAT);     // 跟踪开始时间
            acuTaskLoadParamMap["TrackEndDate"] = item.m_tracking_end_time.toString(DATE_DISPLAY_FORMAT);      // 跟踪结束日期
            acuTaskLoadParamMap["TrackEndTime"] = item.m_tracking_end_time.toString(TIME_DISPLAY_FORMAT);      // 跟踪结束时间
            acuTaskLoadParamMap["TaskEndDate"] = item.m_task_end_time.toString(DATE_DISPLAY_FORMAT);           // 任务结束日期
            acuTaskLoadParamMap["TaskEndTime"] = item.m_task_end_time.toString(TIME_DISPLAY_FORMAT);           // 任务结束时间
            acuTaskLoadParamMap["DTStartDate"] = item.m_dt_starttime.toString(DATE_DISPLAY_FORMAT);            // 数传开始日期
            acuTaskLoadParamMap["DTStartTime"] = item.m_dt_starttime.toString(TIME_DISPLAY_FORMAT);            // 数传开始时间
            acuTaskLoadParamMap["DTEndDate"] = item.m_dt_endtime.toString(DATE_DISPLAY_FORMAT);                // 数传结束日期
            acuTaskLoadParamMap["DTEndTime"] = item.m_dt_endtime.toString(TIME_DISPLAY_FORMAT);                // 数传结束时间

            m_singleCommandHelper.packSingleCommand("Step_ACU_JHXF", m_packCommand, acuDeviceID, acuTaskLoadParamMap);
            waitExecSuccess(m_packCommand);
        }
    }

    auto acuWorkMode = TaskRunCommonHelper::getACUSystemWorkMode(m_taskTimeList.trackingMode);
    if (!acuWorkMode)
    {
        notifyErrorLog(acuWorkMode.msg());
        return;
    }

    QMap<QString, QVariant> unitMap1;
    unitMap1["SystemWorkMode"] = acuWorkMode.value();  //这个参数很重要  1:S 2:Ka遥测 3:Ka数传 4:S+Ka遥测 5:S+Ka数传
    unitMap1["TaskIdentifier"] = m_taskTimeList.masterTaskBz;
    unitMap1["TaskCode"] = m_taskTimeList.masterTaskCode;

    // ACU频率是无效的界面已经屏蔽了
    unitMap1["SCKUpFrequency"] = 0;
    unitMap1["SDownFrequency"] = 0;
    unitMap1["KaCKUpFrequency"] = 0;
    unitMap1["KaCKDownFrequency"] = 0;

    // 获取测角采样率
    // 跟踪模式转换
    QVariant angularSamplRate;
    auto tempWorkMode = TaskRunCommonHelper::trackingToSystemWorkMode(m_taskTimeList.trackingMode);
    if (tempWorkMode != SystemWorkMode::NotDefine)
    {
        MacroCommon::getAngularSamplRate(m_taskTimeList.masterTaskCode, tempWorkMode, m_taskTimeList.trackingPointFreqNo, angularSamplRate);
    }
    if (!angularSamplRate.isValid())
    {
        angularSamplRate = 4;
    }
    //
    unitMap1["AngularSamplRate"] = angularSamplRate;  //测角采样率       1:1次/秒 2:4次/秒 3:10次/秒 4:20次/秒 5:0.2次/秒 6:2次/秒
    unitMap1["AngleDataCorrect"] = 1;                 //角度修正          1:修正 2:不修正 3:自动选择
    unitMap1["AbgularProceCode"] = 1;                 //测角数据标志码	1:PDXP 2:HDLC
    unitMap1["TeleDataCode"] = 2;                     // ACU 1:人工 2:自动

    m_singleCommandHelper.packSingleCommand("Step_ACU_SPU", m_packCommand, acuDeviceID, unitMap1);
    waitExecSuccess(m_packCommand);

    // 参数宏下发完成后运行方式（自动）下发ACU   ACU运行方式设置为自动运行
    m_singleCommandHelper.packSingleCommand("Step_ACU_OperatMode_AutoRun", m_packCommand, acuDeviceID);
    waitExecSuccess(m_packCommand);

    //  4413版
    //    DeviceID acuADeviceID;
    //    acuADeviceID.fromDeviceInfo(0x1011);
    //    acuADeviceID.modeID = 0xFFFF;

    //    DeviceID acuBDeviceID;
    //    acuBDeviceID.fromDeviceInfo(0x1012);
    //    acuBDeviceID.modeID = 0xFFFF;

    //    if (!globalSystemData->deviceIsOnline(acuADeviceID) && !globalSystemData->deviceIsOnline(acuBDeviceID))
    //    {
    //        //如果ACU设备同时离线 ，上报ACU异常
    //        notifyErrorLog("ACU主备设备离线，任务失败");
    //        notify("error");
    //        return;
    //    }
    //    else
    //    {
    //        DeviceID acuMasterID;
    //        acuMasterID.resz();
    //        //如果ACU_A在线
    //        if (globalSystemData->deviceIsOnline(acuADeviceID))
    //        {
    //            //目前只需要下发任务给ACU，多目标每个目标都需要下发对应的ACU计划
    //            acuMasterID = acuADeviceID;
    //        }
    //        //如果ACU_B在线
    //        else if (globalSystemData->deviceIsOnline(acuBDeviceID))
    //        {
    //            acuMasterID = acuBDeviceID;
    //        }
    //        quint64 m_taskBz;
    //        for (auto workMode : m_taskTimeList.m_dataMap.keys())
    //        {
    //            for (auto item : m_taskTimeList.m_dataMap[workMode])
    //            {
    //                QMap<QString, QVariant> acuTaskLoadParamMap;
    //                m_taskBz = item.m_taskBz;
    //                acuTaskLoadParamMap["RaskIdent"] = item.m_taskBz;  // 任务标识
    //                acuTaskLoadParamMap["TaskPrepDate"] =
    //                    converToBCDDate(QDateTime::fromString(item.m_task_ready_start_time, ORIGINALFORMAT));  // 任务准备开始日期
    //                acuTaskLoadParamMap["TaskPrepTime"] =
    //                    converToDayTime(QDateTime::fromString(item.m_task_ready_start_time, ORIGINALFORMAT));  // 任务准备开始时间
    //                acuTaskLoadParamMap["TaskStartDate"] =
    //                    converToBCDDate(QDateTime::fromString(item.m_task_start_time, ORIGINALFORMAT));  // 任务开始日期
    //                acuTaskLoadParamMap["TaskStartTime"] =
    //                    converToDayTime(QDateTime::fromString(item.m_task_start_time, ORIGINALFORMAT));  // 任务开始时间
    //                acuTaskLoadParamMap["TrackStartDate"] =
    //                    converToBCDDate(QDateTime::fromString(item.m_trace_start_time, ORIGINALFORMAT));  // 跟踪开始日期
    //                acuTaskLoadParamMap["TrackStartTime"] =
    //                    converToDayTime(QDateTime::fromString(item.m_trace_start_time, ORIGINALFORMAT));  // 跟踪开始时间
    //                acuTaskLoadParamMap["TrackEndDate"] =
    //                    converToBCDDate(QDateTime::fromString(item.m_tracking_end_time, ORIGINALFORMAT));  // 跟踪结束日期
    //                acuTaskLoadParamMap["TrackEndTime"] =
    //                    converToDayTime(QDateTime::fromString(item.m_tracking_end_time, ORIGINALFORMAT));  // 跟踪结束时间
    //                acuTaskLoadParamMap["TaskEndDate"] = converToBCDDate(QDateTime::fromString(item.m_task_end_time, ORIGINALFORMAT));  //
    //                任务结束日期 acuTaskLoadParamMap["TaskEndTime"] = converToDayTime(QDateTime::fromString(item.m_task_end_time, ORIGINALFORMAT));
    //                // 任务结束时间 cmd.getTaskPlanCmd(array, QString("Step_ACU_JH"), m_currentDeviceName, m_currentDeviceLog, sendIndex,
    //                    m_runningDeviceID,
    //                                       m_runningCmdID, acuTaskLoadParamMap, acuMasterID);
    //                    waitExecSuccess(array);
    //            }
    //        }
    //        MeasurementContrlCmd cmd;
    //        QByteArray array;

    //        QVariant acuTaskIdent;
    //        QVariant acuTaskCode;
    //        for (auto item : m_taskTimeList.m_dataMap)
    //        {
    //            for (auto target : item)
    //            {
    //                if (target.isMaster)
    //                {
    //                    acuTaskCode = target.m_taskCode;
    //                    acuTaskIdent = target.m_taskBz;
    //                }
    //            }
    //        }

    //        QMap<QString, QVariant> unitMap1;
    //        unitMap1["SystemWorkMode"] = 1;  //这个参数很重要  1:S 2:Ka遥测 3:Ka数传 4:S+Ka遥测 5:S+Ka数传
    //        // 1:S 2:Ka遥测 3:Ka低速数传 4:Ka高速数传 5:S+Ka遥测 6:S+Ka低速数传 7:S+Ka高速数传 8:S+Ka遥测+Ka低速数传 9:S+Ka遥测+Ka高速数传

    //        // if(m_taskTimeList.m_dataMap.contains(STTC) && m_taskTimeList.m_dataMap.contains(SKuo2})){}

    //        if (m_taskTimeList.m_gzfsxz == 1)
    //        {
    //            unitMap1["SystemWorkMode"] = 1;
    //        }
    //        if (m_taskTimeList.m_gzfsxz == 2)
    //        {
    //            unitMap1["SystemWorkMode"] = 2;
    //        }
    //        if (m_taskTimeList.m_gzfsxz == 3 || m_taskTimeList.m_gzfsxz == 4)
    //        {
    //            unitMap1["SystemWorkMode"] = 3;
    //        }
    //        if (m_taskTimeList.m_gzfsxz == 5)
    //        {
    //            unitMap1["SystemWorkMode"] = 4;
    //        }
    //        if (m_taskTimeList.m_gzfsxz == 6 || m_taskTimeList.m_gzfsxz == 7)
    //        {
    //            unitMap1["SystemWorkMode"] = 5;
    //        }
    //        if (m_taskTimeList.m_gzfsxz == 8 || m_taskTimeList.m_gzfsxz == 9)
    //        {
    //            unitMap1["SystemWorkMode"] = 6;
    //        }

    //        // 下发ACU参数

    //        unitMap1["TaskIdent"] = acuTaskIdent;
    //        unitMap1["TaskCode"] = acuTaskCode;

    //        if (m_taskTimeList.m_dataMap.contains(STTC))
    //        {
    //            unitMap1["SCKUpFrequency"] = m_taskTimeList.m_upFrequencyMap[STTC];    // S测控上行频率
    //            unitMap1["SDownFrequency"] = m_taskTimeList.m_downFrequencyMap[STTC];  // S下行频率
    //        }

    //        if (m_taskTimeList.m_dataMap.contains(SKuo2))
    //        {
    //            unitMap1["SCKUpFrequency"] = m_taskTimeList.m_upFrequencyMap[SKuo2];    // S测控上行频率
    //            unitMap1["SDownFrequency"] = m_taskTimeList.m_downFrequencyMap[SKuo2];  // S下行频率
    //        }
    //        if (m_taskTimeList.m_dataMap.contains(KaKuo2))
    //        {
    //            unitMap1["KaCKUpFrequency"] = m_taskTimeList.m_upFrequencyMap[KaKuo2];      // Ka测控上行频率
    //            unitMap1["KaCKDownFrequency"] = m_taskTimeList.m_downFrequencyMap[KaKuo2];  // Ka测控下行频率
    //        }

    //        unitMap1["AngularSamplRate"] = 4;  //测角采样率       1:1次/秒 2:4次/秒 3:10次/秒 4:20次/秒 5:0.2次/秒 6:2次/秒
    //        unitMap1["AngleDataCorrect"] = 1;  //角度修正          1:修正 2:不修正 3:自动选择

    //        unitMap1["AbgularProceCode"] = 1;  //测角数据标志码	1:PDXP 2:HDLC
    //        unitMap1["TeleDataCode"] = 2;      // ACU 1:人工 2:自动
    //        cmd.getTaskPlanCmd(array, QString("Step_ACU_UNIT1"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID,
    //        m_runningCmdID,
    //                           unitMap1, acuMasterID);
    //        waitExecSuccess(array);

    //        // 参数宏下发完成后运行方式（自动）下发ACU   ACU运行方式设置为自动运行
    //        cmd.getTaskPlanCmd(array, QString("Step_AC_39_1"), m_currentDeviceName, m_currentDeviceLog, sendIndex, m_runningDeviceID,
    //        m_runningCmdID, {},
    //                           acuMasterID);
    //        waitExecSuccess(array);
    //    }
}

void TaskPreparation::configCCZFKaHS()
{
    if (m_taskTimeList.m_dataMap.contains(KaGS))
    {
        DeviceID satMasterID = m_taskTimeList.m_satDeviceMap[KaGS];
        //下发备机的参数 add by 20210707 下发备机的单元
        auto satSlaveID = satMasterID;
        if (satSlaveID.extenID == 1)
        {
            satSlaveID.extenID = 2;
        }
        else
        {
            satSlaveID.extenID = 1;
        }

        if (!satMasterID.isValid())
        {
            notifyErrorLog("无法找到存储转发设备,系统出错");
        }
        else
        {
            //下发存储单元的参数
            QMap<QString, QVariant> m_saveDataMap;
            //单点频 下发双通道数据
            if (m_taskTimeList.m_dataMap[KaGS].size() == 1)
            {
                // notifyInfoLog("单点频单极化");
                auto taskItem = m_taskTimeList.m_dataMap[KaGS][0];
                auto gsDeviceNameList = m_taskTimeList.getGSDeviceNameList();
                //给map存值
                setCCZFSaveUnitMap(KaGS, gsDeviceNameList[0], 1, taskItem, m_saveDataMap);

                // 下发基带主机存储单元命令
                m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                waitExecSuccess(m_packCommand);

                //下发基带备机存储单元命令  add by 20210707 下发备机的下发单元
                m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[0]);  //设备名称
                m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                waitExecSuccess(m_packCommand);

                //单点频双极化收通道二的数据
                if (m_doubleMap.contains(m_taskTimeList.m_dataMap[KaGS][0].m_taskxuanxiang))
                {
                    notifyInfoLog("双极化");
                    m_saveDataMap["BaseDev"] = gsDeviceNameList[1];  //设备名称
                    m_saveDataMap["BaseChan"] = 2;                   //通道号2

                    // 下发基带主机存储单元命令
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);

                    //下发基带备机存储单元命令  add by 20210707 下发备机的下发单元
                    m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[1]);  //设备名称
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);
                }
                //获取跟踪接收计划号
                // auto dt_transNum = taskItem.m_dt_transNum;
                // auto planSerialNumber = taskItem.m_planSerialNumber;
                //           auto planMgr = getPlanMgrInterface();
                QList<DataTranWorkTask> connectDtTaskList;  //关联的跟踪计划任务
                DataTranWorkTask dataTranWorkTask;
                //            if (planMgr != nullptr)
                //            {
                //                QString errorMsg;
                //                connectDtTaskList = planMgr->getDataTranWorkPlan(planSerialNumber, dt_transNum, errorMsg);
                //            }
                //判断计划是实时接收转发  ，只接收 ，事后转发·
                //当前工作的设备工作计划时实时还是事后
                int dataTranWorkType = 1;  // 1:代表实时 2.代表事后 3.代表只接收
                //如果没有关联的跟踪接收计划号，创建只跟踪计划，
                QMap<QString, QVariant> satPlanParamMap;
                //命令map存取
                setCCZFSaveCmdMap(1, dataTranWorkType, dataTranWorkTask, connectDtTaskList, taskItem, satPlanParamMap);

                //下发计划通道1
                m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                waitExecSuccess(m_packCommand);
                setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);
                //下发备机只接收计划  add by 20210727 下发备机的只接收计划
                satPlanParamMap["OperatMode"] = 3;
                m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                waitExecSuccess(m_packCommand);
                setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);
                if (m_doubleMap.contains(m_taskTimeList.m_dataMap[KaGS][0].m_taskxuanxiang))
                {
                    //下发计划通道2
                    satPlanParamMap["OperatMode"] = dataTranWorkType;
                    satPlanParamMap["LocalNumb"] = 2;  //通道2数据
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);
                    //下发备机只接收计划  add by 20210727 下发备机的只接收计划

                    satPlanParamMap["OperatMode"] = 3;
                    satPlanParamMap["LocalNumb"] = 2;  //通道2数据
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);
                }
            }
            //双点频 双极化 下发双通道数据
            if (m_taskTimeList.m_dataMap[KaGS].size() == 2 && m_doubleMap.contains(m_taskTimeList.m_dataMap[KaGS][0].m_taskxuanxiang) &&
                m_doubleMap.contains(m_taskTimeList.m_dataMap[KaGS][1].m_taskxuanxiang))
            {
                // notifyInfoLog("双点频双极化");
                auto taskItem1 = m_taskTimeList.m_dataMap[KaGS][0];
                auto taskItem2 = m_taskTimeList.m_dataMap[KaGS][1];
                auto gsDeviceNameList = m_taskTimeList.getGSDeviceNameList();

                //给map存值
                setCCZFSaveUnitMap(KaGS, gsDeviceNameList[0], 1, taskItem1, m_saveDataMap);

                // 下发基带主机存储单元命令
                m_saveDataMap["BaseDev"] = gsDeviceNameList[0];  //设备名称
                m_saveDataMap["BaseChan"] = 1;                   //通道号1
                m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                waitExecSuccess(m_packCommand);

                m_saveDataMap["BaseDev"] = gsDeviceNameList[1];  //设备名称
                m_saveDataMap["BaseChan"] = 2;                   //通道号2
                m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                waitExecSuccess(m_packCommand);

                {
                    //通道1
                    m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[0]);  //设备名称
                    m_saveDataMap["BaseChan"] = 1;                                      //通道号1
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);
                    //通道2
                    m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[1]);  //设备名称
                    m_saveDataMap["BaseChan"] = 2;                                      //通道号2
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);
                }
                {
                    //点频1通道1
                    {
                        //获取跟踪接收计划号
                        // auto dt_transNum = taskItem1.m_dt_transNum;
                        // auto planSerialNumber = taskItem1.m_planSerialNumber;
                        //                        auto planMgr = getPlanMgrInterface();
                        QList<DataTranWorkTask> connectDtTaskList;  //关联的跟踪计划任务
                        DataTranWorkTask dataTranWorkTask;
                        //                        if (planMgr != nullptr)
                        //                        {
                        //                            QString errorMsg;
                        //                            connectDtTaskList = planMgr->getDataTranWorkPlan(planSerialNumber, dt_transNum, errorMsg);
                        //                        }
                        //判断计划是实时接收转发  ，只接收 ，事后转发·
                        //当前工作的设备工作计划时实时还是事后
                        int dataTranWorkType = 1;  // 1:代表实时 2.代表事后 3.代表只接收
                        //如果没有关联的跟踪接收计划号，创建只跟踪计划，
                        QMap<QString, QVariant> satPlanParamMap;
                        //命令map存取
                        setCCZFSaveCmdMap(1, dataTranWorkType, dataTranWorkTask, connectDtTaskList, taskItem1, satPlanParamMap);
                        //下发计划通道1
                        satPlanParamMap["LocalNumb"] = 1;  //通道号1
                        m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                        waitExecSuccess(m_packCommand);
                        setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);

                        satPlanParamMap["LocalNumb"] = 2;  //通道号2
                        m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                        waitExecSuccess(m_packCommand);
                        setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);
                    }
                    //点频2通道2
                    {
                        //获取跟踪接收计划号
                        // auto dt_transNum = taskItem2.m_dt_transNum;
                        // auto planSerialNumber = taskItem2.m_planSerialNumber;
                        // auto planMgr = getPlanMgrInterface();
                        QList<DataTranWorkTask> connectDtTaskList;  //关联的跟踪计划任务
                        DataTranWorkTask dataTranWorkTask;
                        //                        if (planMgr != nullptr)
                        //                        {
                        //                            QString errorMsg;
                        //                            connectDtTaskList = planMgr->getDataTranWorkPlan(planSerialNumber, dt_transNum, errorMsg);
                        //                        }
                        //判断计划是实时接收转发  ，只接收 ，事后转发·
                        //当前工作的设备工作计划时实时还是事后
                        int dataTranWorkType = 1;  // 1:代表实时 2.代表事后 3.代表只接收
                        //如果没有关联的跟踪接收计划号，创建只跟踪计划，
                        QMap<QString, QVariant> satPlanParamMap;
                        //命令map存取
                        setCCZFSaveCmdMap(1, dataTranWorkType, dataTranWorkTask, connectDtTaskList, taskItem2, satPlanParamMap);
                        satPlanParamMap["LocalNumb"] = 1;  //通道号1
                        m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                        waitExecSuccess(m_packCommand);
                        setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);

                        satPlanParamMap["LocalNumb"] = 2;  //通道号2
                        m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                        waitExecSuccess(m_packCommand);
                        setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);
                    }
                }
            }
            //双点频 单极化 下发单通道数据
            if ((m_taskTimeList.m_dataMap[KaGS].size() == 2 && (m_simpleMap.contains(m_taskTimeList.m_dataMap[KaGS][0].m_taskxuanxiang) ||
                                                                m_simpleMap.contains(m_taskTimeList.m_dataMap[KaGS][1].m_taskxuanxiang))))
            {
                //  notifyInfoLog("双点频单极化");
                //点频1 使用基带1
                {
                    auto taskItem = m_taskTimeList.m_dataMap[KaGS][0];
                    auto gsDeviceNameList = m_taskTimeList.getGSDeviceNameList();
                    //给map存值
                    setCCZFSaveUnitMap(KaGS, gsDeviceNameList[0], 1, taskItem, m_saveDataMap);

                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);

                    m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[0]);  //设备名称
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT1_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);
                    //下发任务计划
                    // auto dt_transNum = taskItem.m_dt_transNum;
                    // auto planSerialNumber = taskItem.m_planSerialNumber;
                    // auto planMgr = getPlanMgrInterface();
                    QList<DataTranWorkTask> connectDtTaskList;  //关联的跟踪计划任务
                    DataTranWorkTask dataTranWorkTask;
                    //                    if (planMgr != nullptr)
                    //                    {
                    //                        QString errorMsg;
                    //                        connectDtTaskList = planMgr->getDataTranWorkPlan(planSerialNumber, dt_transNum, errorMsg);
                    //                    }
                    //判断计划是实时接收转发  ，只接收 ，事后转发·
                    //当前工作的设备工作计划时实时还是事后
                    int dataTranWorkType = 1;  // 1:代表实时 2.代表事后 3.代表只接收
                    //如果没有关联的跟踪接收计划号，创建只跟踪计划，
                    QMap<QString, QVariant> satPlanParamMap;
                    //命令map存取
                    setCCZFSaveCmdMap(1, dataTranWorkType, dataTranWorkTask, connectDtTaskList, taskItem, satPlanParamMap);

                    //下发计划通道1
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);
                    //下发计划通道2
                    satPlanParamMap["OperatMode"] = 3;
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);
                }
                //点频2使用基带2
                {
                    auto taskItem = m_taskTimeList.m_dataMap[KaGS][1];
                    auto gsDeviceNameList = m_taskTimeList.getGSDeviceNameList();
                    //给map存值
                    setCCZFSaveUnitMap(KaGS, gsDeviceNameList[1], 1, taskItem, m_saveDataMap);

                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satMasterID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);
                    m_saveDataMap["BaseDev"] = replaceBakSatName(gsDeviceNameList[1]);  //设备名称
                    m_singleCommandHelper.packSingleCommand("Step_SAT_UNIT2_SAVE", m_packCommand, satSlaveID, m_saveDataMap);
                    waitExecSuccess(m_packCommand);

                    // auto dt_transNum = taskItem.m_dt_transNum;
                    // auto planSerialNumber = taskItem.m_planSerialNumber;
                    // auto planMgr = getPlanMgrInterface();
                    QList<DataTranWorkTask> connectDtTaskList;  //关联的跟踪计划任务
                    DataTranWorkTask dataTranWorkTask;
                    //                    if (planMgr != nullptr)
                    //                    {
                    //                        QString errorMsg;
                    //                        connectDtTaskList = planMgr->getDataTranWorkPlan(planSerialNumber, dt_transNum, errorMsg);
                    //                    }
                    //判断计划是实时接收转发  ，只接收 ，事后转发·
                    //当前工作的设备工作计划时实时还是事后
                    int dataTranWorkType = 1;  // 1:代表实时 2.代表事后 3.代表只接收
                    //如果没有关联的跟踪接收计划号，创建只跟踪计划，
                    QMap<QString, QVariant> satPlanParamMap;
                    //命令map存取
                    setCCZFSaveCmdMap(2, dataTranWorkType, dataTranWorkTask, connectDtTaskList, taskItem, satPlanParamMap);

                    //下发计划通道1
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satMasterID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satMasterID, satPlanParamMap);
                    //下发计划通道2
                    satPlanParamMap["OperatMode"] = 3;
                    m_singleCommandHelper.packSingleCommand("Step_DSFD_START", m_packCommand, satSlaveID, satPlanParamMap);
                    waitExecSuccess(m_packCommand);
                    setTaskTimeListCCZFCmdMap(satSlaveID, satPlanParamMap);
                }
            }
        }
    }
}

void TaskPreparation::setCCZFSaveUnitMap(SystemWorkMode workMode, const QString& deviceName, int baseChannel, TaskTimeEntity& taskItem,
                                         QMap<QString, QVariant>& m_saveDataMap)
{
    m_saveDataMap["BaseDev"] = deviceName;                                       //设备名称
    m_saveDataMap["BaseChan"] = baseChannel;                                     //通道号1
    m_saveDataMap["DateFormat"] = "B";                                           //日期类型  北京时间
    m_saveDataMap["Date"] = QDate::currentDate().toString(DATE_DISPLAY_FORMAT);  //日期
    m_saveDataMap["Time"] = QTime::currentTime().toString(TIME_DISPLAY_FORMAT);  //时间
    m_saveDataMap["InformCat"] = 1;                                              //信息类别 默认1
    m_saveDataMap["TaskCircle"] = taskItem.m_circle_no;                          //圈次号
    m_saveDataMap["ObjectIden"] = QString::number(taskItem.m_taskBz, 16);        //对象标识 任务标识
    m_saveDataMap["DpNum"] = taskItem.m_dpNum;                                   //目前没有使用，这里使用任务计划的点频

    auto orientation = SystemOrientation::Unkonwn;
    m_taskTimeList.manualMessage.configMacroData.getReceivePolar(workMode, orientation, 1);
    if (orientation == SystemOrientation::Unkonwn)
    {
        orientation = SystemOrientation::LCircumflex;
    }
    else if (orientation == SystemOrientation::LRCircumflex)
    {
        orientation = SystemOrientation::LCircumflex;
    }
    m_saveDataMap["XuanXiang"] = static_cast<int>(orientation);
    //获取基带当前是分路还是和路
    m_saveDataMap["IQOutPut"] = 1;       //和路输出还是分路输出，
    m_saveDataMap["IChannelNum"] = 312;  // I通道号
    m_saveDataMap["QChannelNum"] = 311;  // Q通道号

    //联调是TS，实战时OP，默认都是OP
    if (m_taskTimeList.m_working_mode == "SZ" || m_taskTimeList.m_working_mode == "HL" || m_taskTimeList.m_working_mode == "BJ" ||
        m_taskTimeList.m_working_mode == "ZY")
    {
        m_saveDataMap["PattIdent"] = "OP";  //模式标识
    }
    if (m_taskTimeList.m_working_mode == "LT")
    {
        m_saveDataMap["PattIdent"] = "TS";  //模式标识
    }
}

#include "TaskPlanSql.h"
void TaskPreparation::setCCZFSaveCmdMap(int channel, int dataTranWorkType, DataTranWorkTask& dataTranWorkTask,
                                        QList<DataTranWorkTask> connectDtTaskList, TaskTimeEntity& taskItem, QMap<QString, QVariant>& satPlanParamMap)
{
    // 更新当前任务的状态
    //    auto currentTask = TaskPlanSql::queryPlanByDev(taskMachine->deviceWorkTask());
    //    if (currentTask.success())
    //    {
    //        connectDtTaskList = currentTask.value();
    //    }
    if (connectDtTaskList.size() == 0)
    {
        satPlanParamMap["StartTime"] = taskItem.m_dt_starttime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务开始时间 yyyyMMddhhmmss
        satPlanParamMap["TEndTime"] = taskItem.m_dt_endtime.toString(DATETIME_DISPLAY_FORMAT3);     // 任务结束时间 yyyyMMddhhmmss
        satPlanParamMap["PostTime"] =
            GlobalData::currentDateTime().addSecs(5 * 60).toString(DATETIME_DISPLAY_FORMAT3);  // 事后传输时间 yyyyMMddhhmmss
        if (m_taskTimeList.m_taskCode2SCCenterMap.contains(taskItem.m_taskCode))
        {
            satPlanParamMap["DataTran"] = m_taskTimeList.m_taskCode2SCCenterMap[taskItem.m_taskCode];  // 数据传输中心 以加号连接
        }
        else
        {
            notifyWarningLog("没有匹配到计划对应的数传中心");
        }
        //暂时先注释掉，现在还没有直接获取数传工作计划
        //  notifyWarningLog("没有匹配到对应的数传工作计划");
    }
    else
    {
        dataTranWorkTask = connectDtTaskList[0];  //这里先默认处理一个数传工作计划
        if (dataTranWorkTask.m_working_mode == DataTransMode::RealTime)
        {
            dataTranWorkType = 1;  //实时计划
        }
        else if (dataTranWorkTask.m_working_mode == DataTransMode::Afterwards)
        {
            dataTranWorkType = 2;  //事后计划
        }
        else if (dataTranWorkTask.m_working_mode == DataTransMode::OnlyRecv)
        {
            dataTranWorkType = 3;  //直接接收计划
        }
        else if (dataTranWorkTask.m_working_mode == DataTransMode::FirstRealTime)
        {
            dataTranWorkType = 1;  //首次接收的事后 还是按照实时来处理
        }
        //有数传中心就使用中心数据
        satPlanParamMap["DataTran"] = dataTranWorkTask.m_datatrans_center.trimmed();  // 数据传输中心 以加号连接
        //如果数传工作计划无法提供中心
        if (dataTranWorkTask.m_datatrans_center.isEmpty())
        {
            if (m_taskTimeList.m_taskCode2SCCenterMap.contains(taskItem.m_taskCode))
            {
                satPlanParamMap["DataTran"] = m_taskTimeList.m_taskCode2SCCenterMap[taskItem.m_taskCode];  // 数据传输中心 以加号连接
            }
            else
            {
                notifyWarningLog("没有匹配到计划对应的数传中心");
            }
        }

        satPlanParamMap["StartTime"] = dataTranWorkTask.m_task_start_time.toString(DATETIME_DISPLAY_FORMAT3);  // 任务开始时间 yyyyMMddhhmmss
        satPlanParamMap["TEndTime"] = dataTranWorkTask.m_task_end_time.toString(DATETIME_DISPLAY_FORMAT3);     // 任务结束时间 yyyyMMddhhmmss
        satPlanParamMap["PostTime"] = dataTranWorkTask.m_transport_time.toString(DATETIME_DISPLAY_FORMAT3);    // 事后传输时间    yyyyMMddhhmmss
    }
    satPlanParamMap["OperatMode"] = dataTranWorkType;
    satPlanParamMap["TaskCode"] = taskItem.m_taskCode;                    // 任务代号
    satPlanParamMap["TaskMID"] = QString::number(taskItem.m_taskBz, 16);  // 任务标识
    satPlanParamMap["CircleNumb"] = taskItem.m_circle_no;                 // 圈次编号
    satPlanParamMap["Priority"] = dataTranWorkTask.m_priority;            // 优先级
    satPlanParamMap["LocalNumb"] = channel;                               // 通道1数据
}

void TaskPreparation::setTaskTimeListCCZFCmdMap(DeviceID deviceID, QMap<QString, QVariant>& cmdMap)
{
    m_taskTimeList.m_cczfCmdMap[deviceID].append(cmdMap);
}

void TaskPreparation::deviceInit()
{
    QVariantMap replaceMap;
    replaceMap["UACAddr"] = m_taskTimeList.txUACAddr;

    auto& modeExist = m_taskTimeList.modeExist;
    //设备组合号下发 存盘送数关闭 站址装订下发
    if (modeExist.sttcMode)
    {
        auto deviceId = m_taskTimeList.m_ckDeviceMap[STTC];
        // 关闭基带送数
        m_singleCommandHelper.packSingleCommand("StepCKJD_STTC_SendData_Stop", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJD_STTC_SaveDisk_Stop", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 站址装订
        m_singleCommandHelper.packSingleCommand("StepCJKD_StationAddrBind", m_packCommand, deviceId, STTC, replaceMap);
        waitExecSuccess(m_packCommand);
    }

    if (modeExist.skuo2Mode)
    {
        auto deviceId = m_taskTimeList.m_ckDeviceMap[Skuo2];

        // 关闭基带送数  四个目标全关
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SendData_Stop_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SendData_Stop_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SendData_Stop_3", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SendData_Stop_4", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SaveDisk_Stop_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SaveDisk_Stop_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SaveDisk_Stop_3", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepCKJD_SK2_SaveDisk_Stop_4", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 站址装订
        m_singleCommandHelper.packSingleCommand("StepCJKD_StationAddrBind", m_packCommand, deviceId, Skuo2, replaceMap);
        waitExecSuccess(m_packCommand);
    }

    if (modeExist.sktMode)
    {
        auto deviceId = m_taskTimeList.m_ckDeviceMap[SKT];

        // 关闭基带送数
        m_singleCommandHelper.packSingleCommand("StepCKJD_SKT_SendData_Stop", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepCKJD_SKT_SaveDisk_Stop", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        // 站址装订
        m_singleCommandHelper.packSingleCommand("StepCJKD_StationAddrBind", m_packCommand, deviceId, SKT, replaceMap);
        waitExecSuccess(m_packCommand);
    }

    if (modeExist.xdsMode)
    {
        auto deviceId = m_taskTimeList.m_dsDeviceMap[XDS];

        //关闭基带送数
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Stop_Ka", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Stop_X", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        //关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
    }

    if (modeExist.kadsMode)
    {
        // 基带的工作方式设置为任务方式
        auto deviceId = m_taskTimeList.m_dsDeviceMap[KaDS];

        //关闭基带送数
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Stop_Ka", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDSS_Stop_X", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        //关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
    }

    if (modeExist.kagsMode)
    {
        // 基带的工作方式设置为任务方式
        auto deviceId = m_taskTimeList.m_gsDeviceMap[KaGS];

        //关闭基带送数
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Stop_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDSS_Stop_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        //关闭基带存盘
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Stop_1_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Stop_1_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Stop_2_1", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
        m_singleCommandHelper.packSingleCommand("StepGSJDCP_Stop_2_2", m_packCommand, deviceId);
        waitExecSuccess(m_packCommand);
    }
}

void TaskPreparation::configGZDeviceJWC()
{
    //    QMap<QString, QVariant> gzJWCMap;
    //    gzJWCMap["SF_BANDCHECK"] = 0;
    //    gzJWCMap["KAF_BANDCHECK"] = 0;
    //    gzJWCMap["KADTF_BANDCHECK"] = 0;
    //    gzJWCMap["SF_FRAGCHECK"] = 0;
    //    gzJWCMap["KAF_FRAGCHECK"] = 0;
    //    gzJWCMap["KADTF_FRAGCHECK"] = 0;
    //    gzJWCMap["JWC_BANDSIGNAL1"] = 0;
    //    gzJWCMap["JWC_BANDSIGNAL2"] = 0;
    //    gzJWCMap["JWC_BANDSIGNAL3"] = 0;

    //    for (auto gzDeviceID : m_taskTimeList.m_gzJWCdeviceMap.keys())
    //    {
    //        auto workModelList = m_taskTimeList.m_gzJWCdeviceMap[gzDeviceID];
    //        for (auto workMode : workModelList)
    //        {
    //            if (workMode == SystemWorkMode::STTC || workMode == SystemWorkMode::Skuo2 || workMode == SYTHSMJ || workMode == SYTHWX ||
    //                workMode == SYTHGML || workMode == SYTHSMJK2GZB || workMode == SYTHSMJK2BGZB || workMode == SYTHWXSK2 || workMode == SYTHGMLSK2)
    //            {
    //                gzJWCMap["SF_BANDCHECK"] = gzDeviceID.extenID;
    //                gzJWCMap["SF_FRAGCHECK"] = 1;     // S:1 X:2 Ka:3
    //                gzJWCMap["JWC_BANDSIGNAL1"] = 1;  //测控:1 数传:2
    //            }
    //            if (workMode == KaKuo2)
    //            {
    //                gzJWCMap["KAF_BANDCHECK"] = gzDeviceID.extenID;
    //                gzJWCMap["KAF_FRAGCHECK"] = 3;    // S:1 X:2 Ka:3
    //                gzJWCMap["JWC_BANDSIGNAL2"] = 1;  //测控:1 数传:2
    //            }
    //            if (workMode == KaDS || workMode == KaGS)
    //            {
    //                gzJWCMap["KADTF_BANDCHECK"] = gzDeviceID.extenID;
    //                gzJWCMap["KADTF_FRAGCHECK"] = 3;  // S:1 X:2 Ka:3
    //                gzJWCMap["JWC_BANDSIGNAL3"] = 2;  //测控:1 数传:2
    //            }
    //        }
    //    }
    //    notifyInfoLog(QString("跟踪基带%1,频段%2,模式%3")
    //                      .arg(gzJWCMap["SF_BANDCHECK"].toInt())
    //                      .arg(gzJWCMap["SF_FRAGCHECK"] == 1 ? "S" : "Ka")
    //                      .arg(gzJWCMap["JWC_BANDSIGNAL1"] == 1 ? "测控" : "数传"));

    //    notifyInfoLog(QString("跟踪基带%1,频段%2,模式%3")
    //                      .arg(gzJWCMap["KAF_BANDCHECK"].toInt())
    //                      .arg(gzJWCMap["KAF_FRAGCHECK"] == 1 ? "S" : "Ka")
    //                      .arg(gzJWCMap["JWC_BANDSIGNAL2"] == 1 ? "测控" : "数传"));

    //    notifyInfoLog(QString("跟踪基带%1,频段%2,模式%3")
    //                      .arg(gzJWCMap["KADTF_BANDCHECK"].toInt())
    //                      .arg(gzJWCMap["KADTF_FRAGCHECK"] == 1 ? "S" : "Ka")
    //                      .arg(gzJWCMap["JWC_BANDSIGNAL3"] == 1 ? "测控" : "数传"));
    //    DeviceID gzjwcDeviceID;
    //    gzjwcDeviceID.sysID = 4;
    //    gzjwcDeviceID.devID = 2;
    //    for (auto extendNum = 1; extendNum <= 4; extendNum++)
    //    {
    //        gzjwcDeviceID.extenID = extendNum;
    //        m_singleCommandHelper.packSingleCommand("Step_GZ_JWC", packCommand, gzjwcDeviceID, gzJWCMap);
    //        waitExecSuccess(packCommand);
    //    }
}

void TaskPreparation::configDeviceStartStatus()
{
    //    // UAC地址装订（测控基带）   从站资源获取
    //    QString uacAddr;
    //    if (mGlobalStationResourceData)
    //    {
    //        auto stationResMap = mGlobalStationResourceData->getData();
    //        if (!stationResMap.isEmpty())
    //        {
    //            uacAddr = stationResMap.first().tTCUACAddr;
    //        }
    //    }
    //    QMap<QString, QVariant> uacAddrLoadParam;
    //    uacAddrLoadParam["UACAddr"] = uacAddr.toInt(0, 16);  // 需要转为十六进制
    //    if (uacAddr.isEmpty())
    //    {
    //        notifyErrorLog(QString("UAC地址获取失败,请检查站资源管理是否有本站相关UAC地址"));
    //    }
    //    // 各类基带的工作方式设置为任务方式
    //    // 测控基带设置为任务方式
    //    notifyInfoLog("开始根据工作模式切换");

    //    //设备组合号的map
    //    QMap<int, quint64> deviceNumMap;
    //    for (auto workModeID : m_taskTimeList.m_currentXuanXiangMap.keys())
    //    {
    //        for (auto value : m_taskTimeList.m_dataMap[workModeID])
    //        {
    //            if (value.isMaster)
    //            {
    //                deviceNumMap[workModeID] = value.m_equipNum;
    //            }
    //        }
    //    }

    //    if (m_taskTimeList.m_dataMap.contains(STTC))
    //    {
    //        // 基带的工作方式设置为任务方式
    //        DeviceID deviceId = m_ckDeviceMap[STTC];
    //        m_singleCommandHelper.packSingleCommand("Step_CK_TTC_102_1", packCommand, deviceId, equipNumbMap);
    //        waitExecSuccess(packCommand);
    //        //下发设备组合号
    //        QMap<QString, QVariant> equipNumbMap;
    //        equipNumbMap["SBandNumb"] = deviceNumMap[STTC];
    //        m_singleCommandHelper.packSingleCommand("Step_CK_SBZHH", packCommand, deviceId, equipNumbMap);
    //        waitExecSuccess(packCommand);
    //        //关闭基带送数
    //        m_singleCommandHelper.packSingleCommand("Step_CK_TTC_48_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //关闭基带存盘
    //        m_singleCommandHelper.packSingleCommand("Step_CK_TTC_51_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //站址装订
    //        if (!uacAddr.isEmpty())
    //        {
    //            m_singleCommandHelper.packSingleCommand("Step_CK_TTC_114_1", packCommand, deviceId, uacAddrLoadParam);
    //            waitExecSuccess(packCommand);
    //        }
    //    }
    //    else if (m_taskTimeList.m_dataMap.contains(Skuo2))
    //    {
    //        // 基带的工作方式设置为任务方式
    //        DeviceID deviceId = m_ckDeviceMap[Skuo2];
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_102_1", packCommand, deviceId, equipNumbMap);
    //        waitExecSuccess(packCommand);
    //        //下发设备组合号
    //        QMap<QString, QVariant> equipNumbMap;
    //        equipNumbMap["SBandNumb"] = deviceNumMap[Skuo2];
    //        m_singleCommandHelper.packSingleCommand("Step_CK_SBZHH", packCommand, deviceId, equipNumbMap);
    //        waitExecSuccess(packCommand);

    //        //默认关闭基带4通道送数
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_24_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_24_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_24_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_24_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //默认关闭存盘
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_27_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_27_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_27_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_S_27_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //站址装订
    //        if (!uacAddr.isEmpty())
    //        {
    //            m_singleCommandHelper.packSingleCommand("Step_CK_S_114_1", packCommand, deviceId, uacAddrLoadParam);
    //            waitExecSuccess(packCommand);
    //        }
    //    }
    //    else if (m_taskTimeList.m_dataMap.contains(KaKuo2))
    //    {
    //        // 基带的工作方式设置为任务方式
    //        DeviceID deviceId = m_ckDeviceMap[KaKuo2];

    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_102_1", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //下发设备组合号
    //        QMap<QString, QVariant> equipNumbMap;
    //        equipNumbMap["SBandNumb"] = deviceNumMap[KaKuo2];
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_102_1", packCommand, deviceId, deviceNumMap);
    //        waitExecSuccess(packCommand);

    //        //默认关闭基带4通道送数
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_24_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_24_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_24_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_24_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //默认关闭存盘
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_27_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_27_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_27_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_CK_KA_27_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //站址装订
    //        if (!uacAddr.isEmpty())
    //        {
    //            m_singleCommandHelper.packSingleCommand("Step_CK_KA_114_1", packCommand, deviceId, uacAddrLoadParam);
    //            waitExecSuccess(packCommand);
    //        }
    //    }
    //    if (m_taskTimeList.m_dataMap.contains(XDS))
    //    {
    //        //设置任务方式
    //        DeviceID deviceId = m_dsDeviceMap[XDS];
    //        m_singleCommandHelper.packSingleCommand("Step_DS_102", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //关闭低速基带送数

    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_1_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_1_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        waitExecSuccess(packCommand);
    //        //关闭低速基带存盘
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //    }
    //    if (m_taskTimeList.m_dataMap.contains(KaDS))
    //    {
    //        //设置任务方式
    //        DeviceID deviceId = m_dsDeviceMap[KaDS];
    //        m_singleCommandHelper.packSingleCommand("Step_DS_102", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //关闭低速基带送数
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_1_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_1_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //关闭低速基带存盘
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_LMOD_2_DK_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //    }
    //    if (m_taskTimeList.m_dataMap.contains(KaGS))
    //    {
    //        //设置任务方式
    //        DeviceID deviceId = m_gsDeviceMap[KaGS];
    //        m_singleCommandHelper.packSingleCommand("Step_GS_102", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        //关闭高速基带送数
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_3_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_3_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);

    //        //关闭高速基带存盘
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_4_DK_12", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_4_DK_22", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_4_DK_32", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //        m_singleCommandHelper.packSingleCommand("Step_DBB_4_DK_42", packCommand, deviceId);
    //        waitExecSuccess(packCommand);
    //    }
}

QString TaskPreparation::replaceBakSatName(QString deviceName)
{
    if (deviceName.contains("HDD1"))
    {
        return deviceName.replace("HDD1", "HDD2");
    }
    if (deviceName.contains("HDD2"))
    {
        return deviceName.replace("HDD2", "HDD1");
    }
    if (deviceName.contains("LDD1"))
    {
        return deviceName.replace("LDD1", "LDD2");
    }
    if (deviceName.contains("LDD2"))
    {
        return deviceName.replace("LDD2", "LDD1");
    }

    return deviceName;
}
