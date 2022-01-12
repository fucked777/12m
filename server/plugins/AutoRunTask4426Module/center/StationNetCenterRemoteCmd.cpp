#include "StationNetCenterRemoteCmd.h"
#include "BaseHandler.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "LinkConfigHelper.h"
#include "MessagePublish.h"
#include "PlatformInterface.h"
#include "RedisHelper.h"
#include "SingleCommandHelper.h"
#include "StationNetCenterMessageDefine.h"
#include "TaskCenterDefine.h"
#include "TaskGlobalInfo.h"
#include "ZwzxRemoteControlCmdHelper.h"
#include "SatelliteManagementSerialize.h"
#include <QApplication>
#include <QSettings>

class TaskCenterStationNetMap
{
public:
    static bool getMapValue(QString& bs, const QString& indexStr, int index);
};
bool TaskCenterStationNetMap::getMapValue(QString& bs, const QString& indexStr, int index)
{
    auto filePath = PlatformConfigTools::configBusiness("ZwzxRemoteControl/ZwzxRemoteControl.ini");
    QSettings setting(filePath, QSettings::IniFormat);  //读取配置文件
    auto value = setting.value(QString("%1_%2/value").arg(indexStr).arg(index));
    if (!value.isValid())
    {
        return false;
    }
    bs = value.toString();
    return true;
}
class StationNetCenterRemoteCmdImpl
{
public:
    SNCCmdMap sncCmdMap;
    SingleCommandHelper singleCommandHelper;
    PackCommand packCommand;

public:
    OptionalNotValue packLinkConfigCmd(const SNCCmdNode& sncCmdNode, const QSet<QString>& keyValuePairSet, QList<PackCommand>& packCmdList);
};

OptionalNotValue StationNetCenterRemoteCmdImpl::packLinkConfigCmd(const SNCCmdNode& sncCmdNode, const QSet<QString>& keyValuePairSet,
                                                                  QList<PackCommand>& packCmdList)
{
    QSet<QString> emptySet;
    emptySet << QString("");
    QSet<QSet<QString>> emptySets;
    emptySets << emptySet;

    QList<Command> commandList;
    for (auto& commandGroup : sncCmdNode.cmdGroups)
    {
        // 如果命令组条件为空，表示任何条件都满足，该组下的命令都要发送
        if (commandGroup.conditionSet == emptySets)
        {
            commandList << commandGroup.commands;
            continue;
        }

        for (auto& condition : commandGroup.conditionSet)
        {
            if (keyValuePairSet.contains(condition))  // 满足条件之一就下发
            {
                commandList << commandGroup.commands;
                break;
            }
        }
    }

    if (commandList.isEmpty())
    {
        auto msg = QString("当前任务与中心命令工作模式不匹配，未找到合适的命令");
        return OptionalNotValue(ErrorCode::NotFound, msg);
    }

    // 合并相同命令
    QList<Command> mergeCmdList;
    LinkConfigHelper::mergeCommands(commandList, mergeCmdList);

    // 组包
    for (auto& command : mergeCmdList)
    {
        PackCommand packCmd;
        if (!LinkConfigHelper::packCommand(command, packCmd))
        {
            return OptionalNotValue(ErrorCode::InvalidArgument, packCmd.errorMsg);
        }

        packCmdList << packCmd;
    }

    return OptionalNotValue();
}

StationNetCenterRemoteCmd::StationNetCenterRemoteCmd(const SNCCmdMap& sncCmdMap, QObject* parent)
    : QObject(parent)
    , m_impl(new StationNetCenterRemoteCmdImpl)
{
    m_impl->sncCmdMap = sncCmdMap;
}
StationNetCenterRemoteCmd::~StationNetCenterRemoteCmd()
{
    delete m_impl;
    m_impl = nullptr;
}

void StationNetCenterRemoteCmd::notifyInfoLog(const QString& logStr) { SystemLogPublish::infoMsg(QString("中心指令：%1").arg(logStr)); }
void StationNetCenterRemoteCmd::notifyWarningLog(const QString& logStr) { SystemLogPublish::warningMsg(QString("中心指令：%1").arg(logStr)); }
void StationNetCenterRemoteCmd::notifyErrorLog(const QString& logStr) { SystemLogPublish::errorMsg(QString("中心指令错误：%1").arg(logStr)); }
void StationNetCenterRemoteCmd::notifySpecificTipsLog(const QString& logStr) { SystemLogPublish::specificTipsMsg(QString("中心指令：%1").arg(logStr)); }
void StationNetCenterRemoteCmd::execCmd(const ZwzxRemoteControlCMD& remoteCmd)
{
    /* 获取当前的任务结构 */
    auto runTask = AutoRunTaskStepStatusPublish::getRunningTask();
    auto manualMessage = runTask.msg;
    if (runTask.taskPlanData.deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        auto errMsg = QString("当前无运行中任务拒绝中心指令 参数: %1 结果 %2").arg(int(remoteCmd.pa)).arg(int(remoteCmd.jg));
        notifyWarningLog(errMsg);

        /* 返回执行结果 */
        VoiceAlarmPublish::publish("执行中心命令错误");
        emit signalRemoteControlCmdResult(remoteCmd, ZwzxRemoteControlCmdReplyResult::Refuse);
        return;
    }
    QSet<quint32> taskBZSet;
    SatelliteManagementDataMap satelliteManagementDataMap;
    if(!GlobalData::getSatelliteManagementData(satelliteManagementDataMap))
    {
        notifyWarningLog(QString("获取当前卫星数据错误"));

        /* 返回执行结果 */
        VoiceAlarmPublish::publish("执行中心命令错误");
        emit signalRemoteControlCmdResult(remoteCmd, ZwzxRemoteControlCmdReplyResult::Refuse);
        return;
    }
    for(auto& item : runTask.taskPlanData.deviceWorkTask.m_totalTargetMap)
    {
        if(satelliteManagementDataMap.contains(item.m_task_code))
        {
            taskBZSet << satelliteManagementDataMap.value(item.m_task_code).m_satelliteIdentification.toUInt(nullptr, 16);
        }
    }
    if(!taskBZSet.contains(remoteCmd.header.mid))
    {
        auto errMsg = QString("未找到当前任务卫星代号：%1 拒绝中心指令 当前任务圈号：%2")
                .arg(remoteCmd.header.mid, 0, 16)
                .arg(runTask.taskPlanData.deviceWorkTask.m_plan_serial_number);
        notifyWarningLog(errMsg);

        /* 返回执行结果 */
        VoiceAlarmPublish::publish("执行中心命令错误");
        emit signalRemoteControlCmdResult(remoteCmd, ZwzxRemoteControlCmdReplyResult::Refuse);
        return;
    }

    auto key = remoteCmd.pa << 8 | remoteCmd.jg;
    /* 0000是测试命令没有结果 */
    if (key == 0x0000)
    {
        /*
         * 执行结果 0x00
         * 设备收到后返回应答
         * 无实际控制内容
         */
        return;
    }

    auto sncCmdNode = m_impl->sncCmdMap.value(key);
    /*没找到或者没配置 */
    if (sncCmdNode.cmdGroups.isEmpty())
    {
        auto errMsg = QString("未找到中心命令 参数: %1 结果 %2").arg(int(remoteCmd.pa)).arg(int(remoteCmd.jg));
        notifyErrorLog(errMsg);

        /* 返回执行结果 */
        VoiceAlarmPublish::publish("执行中心命令错误");
        emit signalRemoteControlCmdResult(remoteCmd, ZwzxRemoteControlCmdReplyResult::Error);
        return;
    }

    notifySpecificTipsLog(QString("执行中心命令%1 参数: %2 结果 %3").arg(sncCmdNode.name).arg(int(remoteCmd.pa)).arg(int(remoteCmd.jg)));
    bool modeIsExist = false;
    for (auto& workMode : sncCmdNode.workModeSet)
    {
        if (!manualMessage.configMacroData.configMacroCmdModeMap.contains(workMode))
        {
            continue;
        }

        auto& configMacroCmdList = manualMessage.configMacroData.configMacroCmdModeMap.value(workMode);
        if (!execLinkCommand(sncCmdNode, configMacroCmdList))
        {
            /* 返回执行结果 */
            VoiceAlarmPublish::publish("执行中心命令错误");
            emit signalRemoteControlCmdResult(remoteCmd, ZwzxRemoteControlCmdReplyResult::Error);
            return;
        }
        modeIsExist = true;
    }

    emit signalRemoteControlCmdResult(remoteCmd, modeIsExist ? ZwzxRemoteControlCmdReplyResult::Correct : ZwzxRemoteControlCmdReplyResult::Error);
}
bool StationNetCenterRemoteCmd::waitExecSuccess(const QString& key, const DeviceID& deviceID, uint ttl)
{
    m_impl->singleCommandHelper.packSingleCommand(key, m_impl->packCommand, deviceID);
    return waitExecSuccess(m_impl->packCommand, ttl);
}
bool StationNetCenterRemoteCmd::waitExecSuccess(const PackCommand& packCommand, uint ttl)
{
    if (!packCommand.errorMsg.isEmpty())
    {
        notifyErrorLog(packCommand.errorMsg);
        return false;
    }

    if (packCommand.data.isEmpty())
    {
        notifyErrorLog("系统组包失败，请检查配置");
        return false;
    }

    // 当前命令操作信息
    QString currentCmdOperatorInfo;
    if (packCommand.operatorInfo.isEmpty())
    {
        currentCmdOperatorInfo = QString("%1").arg(packCommand.getSetParamDescValue());
    }
    else
    {
        currentCmdOperatorInfo = packCommand.operatorInfo;
    }

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        notifyErrorLog(QString("%1设备离线,发送命令(%2)失败").arg(GlobalData::getExtensionName(packCommand.deviceID)).arg(currentCmdOperatorInfo));
        return false;
    }

    // 发送数据
    emit signalSendToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    if (packCommand.operatorInfo.isEmpty())
    {
        currentCmdOperatorInfo = QString("%1：%2").arg(GlobalData::getExtensionName(packCommand.deviceID)).arg(packCommand.getSetParamDescValue());
    }
    notifyInfoLog(currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl == 0)
    {
        return true;
    }

    // 从循环读取响应的ID
    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        // 获取命令执行结果
        QString cmdRespnceStr;
        if (RedisHelper::getInstance().getData(redisFindKey, cmdRespnceStr))
        {
            ControlCmdResponse cmdRespnce;
            cmdRespnceStr >> cmdRespnce;

            // 如果是当前发送的命令响应
            if (cmdRespnce.isValid() && cmdRespnce.deviceID == packCommand.deviceID && cmdRespnce.cmdId == packCommand.cmdId &&
                cmdRespnce.cmdType == packCommand.cmdType)
            {
                auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdRespnce.responseResult);
                QString message = QString("%1 (%2)").arg(currentCmdOperatorInfo).arg(cmdExecResultDesc);

                if (cmdRespnce.responseResult == ControlCmdResponseType::Success)
                {
                    notifyInfoLog(message);
                }
                else
                {
                    notifyWarningLog(message);
                }
                // 接收到响应就直接返回
                return true;
            }
        }
        QApplication::processEvents();
        QThread::msleep(500);
        --ttl;
    }
    // 超时未响应
    notifyErrorLog(QString("%1，超时未响应").arg(currentCmdOperatorInfo));
    qWarning() << QString("%1，超时未响应").arg(currentCmdOperatorInfo) << packCommand.data.toHex();

    return false;
}
bool StationNetCenterRemoteCmd::execLinkCommand(const SNCCmdNode& sncCmdNode, const ConfigMacroModeItemMap& configMacroCmdList)
{
    // 配置宏参数命令 组装成QSet，格式为：STTC_LAN_L_SUM:1
    QSet<QString> configMacroCmdParamSet;
    for (auto& configMacroCmd : configMacroCmdList.configMacroCmdMap)
    {
        auto temp = QString("%1:%2").arg(configMacroCmd.id, configMacroCmd.value.toString());
        configMacroCmdParamSet.insert(temp);
    }

    QList<PackCommand> packCmdList;
    auto result = m_impl->packLinkConfigCmd(sncCmdNode, configMacroCmdParamSet, packCmdList);
    if (!result)
    {
        auto msg = QString("中心命令配置错误:%1").arg(result.msg());
        notifyErrorLog(msg);
        return false;
    }

    for (auto& cmd : packCmdList)
    {
        if (!waitExecSuccess(cmd))
        {
            return false;
        }
    }

    return true;
}

/************************************************************************************************************************/
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCommunicationTest(const ZwzxRemoteControlCMD& /*remoteCmd*/, QString& /*errMsg*/)
//{
//    /*
//     * 执行结果 0x00
//     * 设备收到后返回应答
//     * 无实际控制内容
//     */
//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleMeasureControlUpWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    return handleMeasureControlUpWaveSwitch(remoteCmd, true, true, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleMeasureControlUpWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    /*
//     * 功放去电
//     * 基带遥测,测量去调
//     * 上行载波输出禁止
//     */
//    return handleMeasureControlUpWaveSwitch(remoteCmd, false, false, false, errMsg);
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleMeasureControlUpRemoteCtrSwitch01(const ZwzxRemoteControlCMD& /*remoteCmd*/,
//                                                                                                   QString& errMsg)
//{
//    AutoEnd autoEnd(m_impl); /* 自动结束 */

//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QSet<SystemWorkMode> workModeSet;
//    for (auto iter = taskTimeList.m_dataMap.begin(); iter != taskTimeList.m_dataMap.end(); ++iter)
//    {
//        auto workMode = SystemWorkMode(iter.key());
//        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
//        {
//            workModeSet << workMode;
//        }
//    }
//    // 无测控模式
//    if (workModeSet.isEmpty())
//    {
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    // 获取测控基带ID
//    auto result = BaseHandler::getCKJDControlInfo(taskTimeList.manualMessage.configMacroData, *workModeSet.begin());
//    auto msg = result.msg();
//    if (!msg.isEmpty())
//    {
//        notifyErrorLog(msg);
//    }
//    if (!result)
//    {
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    auto ckjdInfo = result.value();

//    // taskTimeList.manualMessage.configMacroData.getCKJDDeviceID()
//    // QMap<int, DeviceID>& m_ckDeviceMap = taskTimeList.m_ckDeviceMap;  //测控模式下的测控基带信息
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息
//    auto& modeExist = taskTimeList.modeExist;
//    if (modeExist.skuo2Mode && modeExist.kakuo2Mode) {}
//    // 测控基带加调遥控信号
//    if (taskTimeList.m_dataMap.contains(STTC))
//    {
//        auto curWorkMode = STTC;
//        /* S功放加电 */
//        auto deviceID = m_hpaDeviceMap[curWorkMode];
//        waitExecSuccess("Step_SHPA_POWER_ON", deviceID);

//        deviceID = m_ckDeviceMap[curWorkMode];
//        /* 上行载波输出 */
//        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", packCommand, deviceID);
//        Wait_Success();

//        /* 遥控加调 */
//        singleCmdHelper.packSingleCommand("StepMACBDC_YK_UP", packCommand, deviceID);
//        Wait_Success();

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }
//    // 扩跳没有遥控加调  不知道怎么弄 20210804 wp??
//    //    else if (taskTimeList.m_dataMap.contains(SKT))
//    //    {
//    //        auto curWorkMode = SKT;
//    //        /* S功放加电 */
//    //        deviceID = m_hpaDeviceMap[curWorkMode];
//    //        singleCmdHelper.packSingleCommand("Step_SHPA_POWER_ON", packCommand, deviceID);
//    //        Wait_Success();

//    //        deviceID = m_ckDeviceMap[curWorkMode];
//    //        /* 上行载波输出 */
//    //        singleCmdHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", packCommand, deviceID);
//    //        Wait_Success();

//    //        /* 遥控加调 */
//    //        singleCmdHelper.packSingleCommand("StepMACBDC_YK_UP", packCommand, deviceID);
//    //        Wait_Success();

//    //        return ZwzxRemoteControlCmdReplyResult::Correct;
//    //    }
//    else if (taskTimeList.m_dataMap.contains(Skuo2))
//    {
//        auto curWorkMode = Skuo2;
//        /* S功放加电 */
//        auto deviceID = m_hpaDeviceMap[curWorkMode];
//        mSingleCommandHelper.packSingleCommand("Step_SHPA_POWER_ON", mPackCommand, deviceID);
//        Wait_Success();

//        deviceID = m_ckDeviceMap[curWorkMode];
//        /* 获取当前基带的目标数量 */
//        auto numResult = ZwzxRemoteControlCmdHelper::getMACBCurTargetNum(deviceID, curWorkMode);
//        if (!numResult)
//        {
//            errMsg = numResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }

//        /*
//         * 在4413上无法发送过程控制命令
//         * 因为这个命令在关闭的时候有问题
//         * 会导致总的载波一起关闭
//         *
//         * 这里是开 只管遥控不管测量
//         *
//         * 20210804 因为这里的命令没有考虑单元的多目标情况亮哥他代码好像没写
//         * 所以这里直接写过程控制
//         */

//        auto num = numResult.value();
//        for (int i = 1; i <= num; ++i)
//        {
//            /* 上行载波输出
//             */
//            /* 通道选择
//             * uvalue="1" desc="遥控通道1"/>
//             * uvalue="2" desc="遥控通道2"/>
//             * uvalue="3" desc="遥控通道3"/>
//             * uvalue="4" desc="遥控通道4"/>
//             * uvalue="5" desc="测量通道1"/>
//             * uvalue="6" desc="测量通道2"/>
//             * uvalue="7" desc="测量通道3"/>
//             * uvalue="8" desc="测量通道4"/>
//             */
//            QVariantMap tempTargetMap;
//            tempTargetMap["ChanSelect"] = i;
//            singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", packCommand, deviceID, tempTargetMap);
//            Wait_Success();

//            /* 伪码数据加调 */
//            singleCmdHelper.packSingleCommand("StepMACBDC_SK2_YK_UP", packCommand, deviceID, tempTargetMap);
//            Wait_Success();
//        }

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }

//    errMsg = "当前基带模式不正确";
//    return ZwzxRemoteControlCmdReplyResult::Error;
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleMeasureControlUpRemoteCtrSwitch02(const ZwzxRemoteControlCMD& /*remoteCmd*/,
//                                                                                                   QString& errMsg)
//{
//    AutoEnd autoEnd(m_impl); /* 自动结束 */
//    /*
//     * 当前通道,遥控去调,如果测量支路去调则关上行
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_ckDeviceMap = taskTimeList.m_ckDeviceMap;    //测控模式下的测控基带信息
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    // 测控基带加调遥控信号
//    if (taskTimeList.m_dataMap.contains(STTC))
//    {
//        auto curWorkMode = STTC;

//        deviceID = m_ckDeviceMap[curWorkMode];
//        /* 先关遥控 */
//        singleCmdHelper.packSingleCommand("StepMACBDC_YK_DOWN", packCommand, deviceID);
//        Wait_Success();

//        /* 获取测距加调状态 */

//        /* 获取当前通道的,测量伪码 数据加调情况 */
//        auto cjjtResult = ZwzxRemoteControlCmdHelper::getMACBBZCJJT(deviceID, curWorkMode);
//        if (!cjjtResult)
//        {
//            errMsg = cjjtResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }
//        /* 加调了,就不能关闭载波输出,此处认为成功就行 */
//        if (cjjtResult.value())
//        {
//            return ZwzxRemoteControlCmdReplyResult::Correct;
//        }

//        /* 没有加调关闭载波 */
//        /* 上行载波输出 */
//        singleCmdHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput_STOP", packCommand, deviceID);
//        Wait_Success();

//        /* 关闭S功放 */
//        deviceID = m_hpaDeviceMap[curWorkMode];
//        singleCmdHelper.packSingleCommand("Step_SHPA_POWER_OFF", packCommand, deviceID);
//        Wait_Success();

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }
//    else if (taskTimeList.m_dataMap.contains(Skuo2))
//    {
//        auto curWorkMode = Skuo2;
//        int count = 0; /* 计数如果此计数最后不为0,则证明有通道未关闭上行,所以不能关闭功放 */
//        deviceID = m_ckDeviceMap[curWorkMode];

//        /* 获取当前基带的目标数量 */
//        auto numResult = ZwzxRemoteControlCmdHelper::getMACBCurTargetNum(deviceID, curWorkMode);
//        if (!numResult)
//        {
//            errMsg = numResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }

//        /* 获取基带的输出电平 */
//        auto carrierLevelRes = ZwzxRemoteControlCmdHelper::getMACBOutputLev(deviceID, curWorkMode);
//        if (!carrierLevelRes)
//        {
//            errMsg = numResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }

//        auto carrierLevel = carrierLevelRes.value();
//        auto num = numResult.value();

//        for (int i = 1; i <= num; ++i)
//        {
//            /* 关遥控
//             */
//            /* 通道选择
//             * uvalue="1" desc="遥控通道1"/>
//             * uvalue="2" desc="遥控通道2"/>
//             * uvalue="3" desc="遥控通道3"/>
//             * uvalue="4" desc="遥控通道4"/>
//             * uvalue="5" desc="测量通道1"/>
//             * uvalue="6" desc="测量通道2"/>
//             * uvalue="7" desc="测量通道3"/>
//             * uvalue="8" desc="测量通道4"/>
//             * uvalue="9" desc="全部通道"/>
//             * uvalue="10" desc="全部遥测通道"/>
//             * uvalue="11" desc="全部测量通道"/>
//             */
//            QVariantMap tempTargetMap;
//            tempTargetMap["ChanSelect"] = i;
//            tempTargetMap["OutputLev"] = carrierLevel;
//            singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput_STOP", packCommand, deviceID, tempTargetMap);
//            Wait_Success();

//            /* 遥控伪码数据去调 */
//            singleCmdHelper.packSingleCommand("StepMACBDC_SK2_YK_DOWN", packCommand, deviceID, tempTargetMap);
//            Wait_Success();

//            /* 获取当前通道的,测量伪码 数据加调情况 载波输出 */
//            /* 测量伪码加调 */
//            auto clwmjtResult = ZwzxRemoteControlCmdHelper::getMACBKPCLWMJT(deviceID, curWorkMode, i);
//            if (!clwmjtResult)
//            {
//                errMsg = clwmjtResult.msg();
//                return ZwzxRemoteControlCmdReplyResult::Error;
//            }
//            /* 测量数据加调 */
//            auto cltdscResult = ZwzxRemoteControlCmdHelper::getMACBCLTDSCKZ(deviceID, curWorkMode, i);
//            if (!cltdscResult)
//            {
//                errMsg = cltdscResult.msg();
//                return ZwzxRemoteControlCmdReplyResult::Error;
//            }

//            /* 测量通道输出 */
//            auto clsjjtResult = ZwzxRemoteControlCmdHelper::getMACBKPCLSJJT(deviceID, curWorkMode, i);
//            if (!clsjjtResult)
//            {
//                errMsg = clsjjtResult.msg();
//                return ZwzxRemoteControlCmdReplyResult::Error;
//            }
//            /* 如果有效,则计数+1 */
//            if (cltdscResult.value() & (clwmjtResult.value() | clsjjtResult.value()))
//            {
//                ++count;
//            }
//        }

//        /* 证明全关了,这里也关闭功放 */
//        /* 如果count<=0 证明测量已经无效了,直接关闭就行
//         * 否则不管总载波输出控制
//         */

//        if (count <= 0)
//        {
//            QVariantMap tempTargetMap;
//            tempTargetMap["ChanSelect"] = 9;
//            singleCmdHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput_STOP", packCommand, deviceID, tempTargetMap);
//            Wait_Success();

//            /* S功放去电 */
//            deviceID = m_hpaDeviceMap[curWorkMode];
//            singleCmdHelper.packSingleCommand("Step_SHPA_POWER_OFF", packCommand, deviceID);
//            Wait_Success();
//        }

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }

//    errMsg = "当前基带模式不正确";
//    return ZwzxRemoteControlCmdReplyResult::Error;
//}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch01(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch02(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch03(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch04(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch05(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch06(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch11(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch12(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleUpDataTransWaveSwitch13(const ZwzxRemoteControlCMD& /* remoteCmd */) {}
//// void StationNetCenterRemoteCmd::handleXUpDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd) {}
//// void StationNetCenterRemoteCmd::handleKaUpDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd) {}
//// void StationNetCenterRemoteCmd::handleSDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd) {}
//// void StationNetCenterRemoteCmd::handleXDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd) {}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaUpDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    return handleKaUpDataTransWaveSwitch(remoteCmd, true, true, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaUpDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    /*
//     * 功放去电
//     * 基带遥测去调
//     * 上行载波输出禁止
//     */
//    return handleKaUpDataTransWaveSwitch(remoteCmd, false, false, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaUpDataTransWaveSwitch03(const ZwzxRemoteControlCMD& /*remoteCmd*/, QString&
// errMsg)
//{
//    /*
//     * 当前通道，如果测量支路去调则关上行 遥控去调
//     * 否则 遥控去调
//     */
//    AutoEnd autoEnd(m_impl); /* 自动结束 */

//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_ckDeviceMap = taskTimeList.m_ckDeviceMap;    //测控模式下的测控基带信息
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    auto curWorkMode = KaKuo2;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前基带模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    int count = 0; /* 计数如果此计数最后不为0,则证明有通道未关闭上行,所以不能关闭功放 */
//    deviceID = m_ckDeviceMap[curWorkMode];

//    /* 获取当前基带的目标数量 */
//    auto numResult = ZwzxRemoteControlCmdHelper::getMACBCurTargetNum(deviceID, curWorkMode);
//    if (!numResult)
//    {
//        errMsg = numResult.msg();
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    /* 获取基带的输出电平 */
//    auto carrierLevelRes = ZwzxRemoteControlCmdHelper::getMACBOutputLev(deviceID, curWorkMode);
//    if (!carrierLevelRes)
//    {
//        errMsg = numResult.msg();
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    auto carrierLevel = carrierLevelRes.value();
//    auto num = numResult.value();

//    for (int i = 1; i <= num; ++i)
//    {
//        /* 关遥控
//         */
//        /* 通道选择
//         * uvalue="1" desc="遥控通道1"/>
//         * uvalue="2" desc="遥控通道2"/>
//         * uvalue="3" desc="遥控通道3"/>
//         * uvalue="4" desc="遥控通道4"/>
//         * uvalue="5" desc="测量通道1"/>
//         * uvalue="6" desc="测量通道2"/>
//         * uvalue="7" desc="测量通道3"/>
//         * uvalue="8" desc="测量通道4"/>
//         * uvalue="9" desc="全部通道"/>
//         * uvalue="10" desc="全部遥测通道"/>
//         * uvalue="11" desc="全部测量通道"/>
//         */
//        QVariantMap tempTargetMap;
//        tempTargetMap["ChanSelect"] = i;
//        tempTargetMap["OutputLev"] = carrierLevel;
//        singleCmdHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput_STOP", packCommand, deviceID, tempTargetMap);
//        Wait_Success();

//        /* 遥控伪码数据去调 */
//        singleCmdHelper.packSingleCommand("StepMACBDC_KaK2_YK_DOWN", packCommand, deviceID, tempTargetMap);
//        Wait_Success();

//        /* 获取当前通道的,测量伪码 数据加调情况 载波输出 */
//        /* 测量伪码加调 */
//        auto clwmjtResult = ZwzxRemoteControlCmdHelper::getMACBKPCLWMJT(deviceID, curWorkMode, i);
//        if (!clwmjtResult)
//        {
//            errMsg = clwmjtResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }
//        /* 测量数据加调 */
//        auto cltdscResult = ZwzxRemoteControlCmdHelper::getMACBCLTDSCKZ(deviceID, curWorkMode, i);
//        if (!cltdscResult)
//        {
//            errMsg = cltdscResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }

//        /* 测量通道输出 */
//        auto clsjjtResult = ZwzxRemoteControlCmdHelper::getMACBKPCLSJJT(deviceID, curWorkMode, i);
//        if (!clsjjtResult)
//        {
//            errMsg = clsjjtResult.msg();
//            return ZwzxRemoteControlCmdReplyResult::Error;
//        }
//        /* 如果有效,则计数+1 */
//        if (cltdscResult.value() & (clwmjtResult.value() | clsjjtResult.value()))
//        {
//            ++count;
//        }
//    }

//    /* 证明全关了,这里也关闭功放 */
//    /* 如果count<=0 证明测量已经无效了,直接关闭就行
//     * 否则不管总载波输出控制
//     */
//    QVariantMap tempTargetMap;
//    tempTargetMap["ChanSelect"] = 9;
//    singleCmdHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput_STOP", packCommand, deviceID, tempTargetMap);
//    Wait_Success();

//    /* Ka功放去电 */
//    deviceID = m_hpaDeviceMap[curWorkMode];

//    // 截止20210825
//    // Ka的功放无加去电命令 当前命令是隐藏的不能用
//    // 当前的加去电操作是直接控制射频输出来实现的 所以这里直接注释掉
//    // 使用控制功率输出的就行
//    // singleCmdHelper.packSingleCommand("StepKaHPA_OFF_POWER", packCommand, deviceID);
//    // Wait_Success();

//    singleCmdHelper.packSingleCommand("StepKaHPA_Power_Forbid", packCommand, deviceID);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDataTransWaveSwitch01(const ZwzxRemoteControlCMD& /*remoteCmd*/, QString&
// errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_dsDeviceMap = taskTimeList.m_dsDeviceMap;    //低速
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    auto curWorkMode = KaDS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    deviceID = m_dsDeviceMap[curWorkMode];
//    /* 载波输出 */
//    singleCmdHelper.packSingleCommand("Step_DSJD_CarrOut_On", packCommand, deviceID);
//    Wait_Success();

//    deviceID = m_hpaDeviceMap[curWorkMode];

//    // 截止20210825
//    // Ka的功放无加去电命令 当前命令是隐藏的不能用
//    // 当前的加去电操作是直接控制射频输出来实现的 所以这里直接注释掉
//    // 有一个控制功率输出的就行
//    /* 数传功放开 */
//    // singleCmdHelper.packSingleCommand("StepKaSCHPA_ON_POWER", packCommand, deviceID);
//    // Wait_Success();

//    singleCmdHelper.packSingleCommand("StepKaSCHPA_Power_Allow", packCommand, deviceID);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDataTransWaveSwitch02(const ZwzxRemoteControlCMD& /*remoteCmd*/, QString&
// errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_dsDeviceMap = taskTimeList.m_dsDeviceMap;    //低速
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    auto curWorkMode = KaDS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    deviceID = m_dsDeviceMap[curWorkMode];

//    deviceID = m_dsDeviceMap[curWorkMode];
//    /* 载波输出 */
//    singleCmdHelper.packSingleCommand("Step_DSJD_CarrOut_Off", packCommand, deviceID);
//    Wait_Success();

//    deviceID = m_hpaDeviceMap[curWorkMode];

//    // 截止20210825
//    // Ka的功放无加去电命令 当前命令是隐藏的不能用
//    // 当前的加去电操作是直接控制射频输出来实现的 所以这里直接注释掉
//    // 有一个控制功率输出的就行
//    /* 数传功放开 */
//    // singleCmdHelper.packSingleCommand("StepKaSCHPA_OFF_POWER", packCommand, deviceID);
//    // Wait_Success();

//    singleCmdHelper.packSingleCommand("StepKaSCHPA_Power_Forbid", packCommand, deviceID);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch11(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchAll(remoteCmd, false, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch12(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchAll(remoteCmd, false, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch13(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 1, false, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch14(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 1, false, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch15(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 2, false, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleXDownDataTransWaveSwitch16(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 2, false, false, errMsg);
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchAll(remoteCmd, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchAll(remoteCmd, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x01, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x01, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch05(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x02, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch06(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x02, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch07(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x03, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch08(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x03, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch09(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x04, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch10(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleKaDownDataTransWaveSwitchItem(remoteCmd, 0x04, false, errMsg);
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch11(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchAll(remoteCmd, true, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch12(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchAll(remoteCmd, true, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch13(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 1, true, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch14(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 1, true, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch15(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 2, true, true, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitch16(const ZwzxRemoteControlCMD& remoteCmd, QString&
// errMsg)
//{
//    return handleDSDownDataTransWaveSwitchItem(remoteCmd, 2, true, false, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCKCenterDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "CXZX", 0x01, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCKCenterDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "CXZX", 0x02, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCKCenterDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "CXZX", 0x03, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCKCenterDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "CXZX", 0x04, errMsg);
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDTCenterDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "SCZX", 0x01, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDTCenterDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "SCZX", 0x02, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDTCenterDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "SCZX", 0x03, errMsg);
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDTCenterDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd,
//                                                                                                   QString& errMsg)
//{
//    return handleCenterDownDataTransWaveSwitch(remoteCmd, "SCZX", 0x04, errMsg);
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleMeasureControlUpWaveSwitch(const ZwzxRemoteControlCMD& /*remoteCmd*/, bool
// hpPower,
//                                                                                            bool carrierOutput, bool addRemove, QString& errMsg)
//{
//    AutoEnd autoEnd(m_impl); /* 自动结束 */
//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_ckDeviceMap = taskTimeList.m_ckDeviceMap;    //测控模式下的测控基带信息
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    // 测控基带加调遥控信号
//    if (taskTimeList.m_dataMap.contains(STTC))
//    {
//        auto curWorkMode = STTC;
//        /* S功放加电 */
//        deviceID = m_hpaDeviceMap[curWorkMode];
//        /* ON OFF */
//        singleCmdHelper.packSingleCommand(QString("Step_SHPA_POWER_%1").arg(hpPower ? "ON" : "OFF"), packCommand, deviceID);
//        Wait_Success();

//        deviceID = m_ckDeviceMap[curWorkMode];

//        /* 上行载波输出 */
//        singleCmdHelper.packSingleCommand(QString("StepSTTC_UplinkCarrierOutput%1").arg(carrierOutput ? "" : "_STOP"), packCommand, deviceID);
//        Wait_Success();

//        /* 测距加调 */
//        singleCmdHelper.packSingleCommand(QString("StepMACBDC_CJ_%1").arg(addRemove ? "UP" : "DOWN"), packCommand, deviceID);
//        Wait_Success();

//        /* 遥控加调 */
//        singleCmdHelper.packSingleCommand(QString("StepMACBDC_YK_%1").arg(addRemove ? "UP" : "DOWN"), packCommand, deviceID);
//        Wait_Success();

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }
//    else if (taskTimeList.m_dataMap.contains(Skuo2))
//    {
//        auto curWorkMode = Skuo2;
//        /* S功放加电 */
//        deviceID = m_hpaDeviceMap[curWorkMode];
//        /* ON OFF */
//        singleCmdHelper.packSingleCommand(QString("Step_SHPA_POWER_%1").arg(hpPower ? "ON" : "OFF"), packCommand, deviceID);
//        Wait_Success();

//        deviceID = m_ckDeviceMap[curWorkMode];

//        /* 上行载波输出
//         */
//        /* 通道选择
//         * uvalue="1" desc="遥控通道1"/>
//         * uvalue="2" desc="遥控通道2"/>
//         * uvalue="3" desc="遥控通道3"/>
//         * uvalue="4" desc="遥控通道4"/>
//         * uvalue="5" desc="测量通道1"/>
//         * uvalue="6" desc="测量通道2"/>
//         * uvalue="7" desc="测量通道3"/>
//         * uvalue="8" desc="测量通道4"/>
//         * uvalue="9" desc="全部通道"/>
//         * uvalue="10" desc="全部遥测通道"/>
//         * uvalue="11" desc="全部测量通道"/>
//         */
//        QVariantMap tempTargetMap;
//        tempTargetMap["ChanSelect"] = 9;
//        singleCmdHelper.packSingleCommand(QString("StepSkuo2_UplinkCarrierOutput%1").arg(carrierOutput ? "" : "_STOP"), packCommand, deviceID,
//                                          tempTargetMap);
//        Wait_Success();

//        /* 伪码数据加调 */
//        singleCmdHelper.packSingleCommand(QString("StepMACBDC_SK2_YK_%1").arg(addRemove ? "UP" : "DOWN"), packCommand, deviceID, tempTargetMap);
//        Wait_Success();

//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }

//    errMsg = "当前基带模式不正确";
//    return ZwzxRemoteControlCmdReplyResult::Error;
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaUpDataTransWaveSwitch(const ZwzxRemoteControlCMD& /*remoteCmd*/, bool hpPower,
//                                                                                         bool carrierOutput, bool addRemove, QString& errMsg)
//{
//    /*
//     * 功放加电
//     * 基带遥测,测量加调
//     * 上行载波输出
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_ckDeviceMap = taskTimeList.m_ckDeviceMap;    //测控模式下的测控基带信息
//    QMap<int, DeviceID>& m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;  //功放的信息

//    auto curWorkMode = KaKuo2;

//    // 测控基带加调遥控信号
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前基带模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    /* Ka功放射频输出 */
//    deviceID = m_hpaDeviceMap[curWorkMode];
//    /* ON OFF */
//    singleCmdHelper.packSingleCommand(QString("StepKaHPA_%1_POWER").arg(hpPower ? "ON" : "OFF"), packCommand, deviceID);
//    Wait_Success();

//    deviceID = m_ckDeviceMap[curWorkMode];

//    /* 上行载波输出
//     */
//    /* 通道选择
//     * uvalue="1" desc="遥控通道1"/>
//     * uvalue="2" desc="遥控通道2"/>
//     * uvalue="3" desc="遥控通道3"/>
//     * uvalue="4" desc="遥控通道4"/>
//     * uvalue="5" desc="测量通道1"/>
//     * uvalue="6" desc="测量通道2"/>
//     * uvalue="7" desc="测量通道3"/>
//     * uvalue="8" desc="测量通道4"/>
//     * uvalue="9" desc="全部通道"/>
//     * uvalue="10" desc="全部遥测通道"/>
//     * uvalue="11" desc="全部测量通道"/>
//     */
//    QVariantMap tempTargetMap;
//    tempTargetMap["ChanSelect"] = 9;
//    singleCmdHelper.packSingleCommand(QString("StepKakuo2_UplinkCarrierOutput%1").arg(carrierOutput ? "" : "_STOP"), packCommand, deviceID,
//                                      tempTargetMap);
//    Wait_Success();

//    /* 伪码数据加调 */
//    singleCmdHelper.packSingleCommand(QString("StepMACBDC_KaK2_YK_%1").arg(addRemove ? "UP" : "DOWN"), packCommand, deviceID, tempTargetMap);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDSDownDataTransWaveSwitchAll(const ZwzxRemoteControlCMD& /*remoteCmd*/, bool isKa,
//                                                                                              bool start, QString& errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_dsDeviceMap = taskTimeList.m_dsDeviceMap;  //低速

//    auto curWorkMode = KaDS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    /* 先获取点频数量 */
//    /* X只有两个点频
//     * 低速A 分机号1 和低速B  分机号 2
//     * 策略是获取当前设备是点频1
//     * 那另一个就是点频2
//     *
//     * 送数有两个通道
//     * 通道1 是Ka
//     * 通道2 是X
//     */
//    auto dpNum = taskTimeList.m_dataMap[curWorkMode].size();
//    switch (dpNum)
//    {
//    case 2:
//    {
//        deviceID = m_dsDeviceMap[curWorkMode];
//        if (deviceID.extenID == 2)
//        {
//            deviceID.extenID = 1;
//        }
//        else if (deviceID.extenID == 1)
//        {
//            deviceID.extenID = 2;
//        }

//        /* 送数开关 */
//        auto cmdStep = QString("StepDSJDSS_%1_%2").arg(start ? "Start" : "Stop", isKa ? "Ka" : "X");
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//    }
//    case 1:
//    {
//        /* 送数开关 */
//        deviceID = m_dsDeviceMap[curWorkMode];
//        auto cmdStep = QString("StepDSJDSS_%1_%2").arg(start ? "Start" : "Stop", isKa ? "Ka" : "X");
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }
//    }

//    errMsg = "当前点频错误";
//    return ZwzxRemoteControlCmdReplyResult::Error;
//}
// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitchAll(const ZwzxRemoteControlCMD& /* remoteCmd */, bool
// start,
//                                                                                              QString& errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_gsDeviceMap = taskTimeList.m_gsDeviceMap;  //低速

//    auto curWorkMode = KaGS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    /* 先获取点频数量 */
//    /* Ka有4个点频
//     * 高速A 分机号1 和高速B  分机号 2
//     * 策略是获取当前设备是点频1 点频2
//     * 那另一个就是点频3 4
//     *
//     * 送数有两个通道都是Ka的
//     * 当前的设备是点频1 点频2 另一个就是点频3 点频4
//     */
//    auto dpNum = taskTimeList.m_dataMap[curWorkMode].size();
//    switch (dpNum)
//    {
//    case 4:
//    {
//        deviceID = m_gsDeviceMap[curWorkMode];
//        if (deviceID.extenID == 2)
//        {
//            deviceID.extenID = 1;
//        }
//        else if (deviceID.extenID == 1)
//        {
//            deviceID.extenID = 2;
//        }

//        /* 送数开关 */
//        auto cmdStep = QString("StepGSJDSS_%1_%2").arg(start ? "Start" : "Stop").arg(0x02);
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//    }
//    case 3:
//    {
//        /* 送数开关 */
//        auto cmdStep = QString("StepGSJDSS_%1_%2").arg(start ? "Start" : "Stop").arg(0x01);
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//    }
//    case 2:
//    {
//        deviceID = m_gsDeviceMap[curWorkMode];
//        /* 送数开关 */
//        auto cmdStep = QString("StepGSJDSS_%1_%2").arg(start ? "Start" : "Stop").arg(0x02);
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//    }
//    case 1:
//    {
//        /* 送数开关 */
//        auto cmdStep = QString("StepGSJDSS_%1_%2").arg(start ? "Start" : "Stop").arg(0x01);
//        singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//        Wait_Success();
//        return ZwzxRemoteControlCmdReplyResult::Correct;
//    }
//    }

//    errMsg = "当前点频错误";
//    return ZwzxRemoteControlCmdReplyResult::Error;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleDSDownDataTransWaveSwitchItem(const ZwzxRemoteControlCMD& /* remoteCmd */, int dp,
//                                                                                               bool isKa, bool start, QString& errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_dsDeviceMap = taskTimeList.m_dsDeviceMap;  //低速

//    auto curWorkMode = KaDS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    auto dpNum = taskTimeList.m_dataMap[curWorkMode].size();
//    if (dpNum < dp)
//    {
//        errMsg = "当前点频错误";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    deviceID = m_dsDeviceMap[curWorkMode];

//    if (dp != 1)
//    {
//        if (deviceID.extenID == 2)
//        {
//            deviceID.extenID = 1;
//        }
//        else if (deviceID.extenID == 1)
//        {
//            deviceID.extenID = 2;
//        }
//    }

//    /* 送数开关 */
//    auto cmdStep = QString("StepDSJDSS_%1_%2").arg(start ? "Start" : "Stop", isKa ? "Ka" : "X");
//    singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleKaDownDataTransWaveSwitchItem(const ZwzxRemoteControlCMD& /* remoteCmd */, int dp,
//                                                                                               bool start, QString& errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();
//    QMap<int, DeviceID>& m_gsDeviceMap = taskTimeList.m_gsDeviceMap;
//    // QMap<int, DeviceID>& m_satDeviceMap = taskTimeList.m_satDeviceMap;

//    auto curWorkMode = KaGS;
//    if (!taskTimeList.m_dataMap.contains(curWorkMode))
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    /* 20210420 现在低速高速基带的送数控制是默认常送无法通过命令停止
//     * 现在存转的策略是当停止存储和控制后会停止
//     */
//    auto dpNum = taskTimeList.m_dataMap[curWorkMode].size();
//    if (dpNum < dp)
//    {
//        errMsg = "当前点频错误";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    deviceID = m_gsDeviceMap[curWorkMode];

//    if (dp == 3 || dp == 4)
//    {
//        dp -= 2;
//        if (deviceID.extenID == 2)
//        {
//            deviceID.extenID = 1;
//        }
//        else if (deviceID.extenID == 1)
//        {
//            deviceID.extenID = 2;
//        }
//    }

//    /* 送数开关 */
//    auto cmdStep = QString("StepGSJDSS_%1_%2").arg(start ? "Start" : "Stop").arg(dp);
//    singleCmdHelper.packSingleCommand(cmdStep, packCommand, deviceID);
//    Wait_Success();

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}

// ZwzxRemoteControlCmdReplyResult StationNetCenterRemoteCmd::handleCenterDownDataTransWaveSwitch(const ZwzxRemoteControlCMD& /*remoteCmd*/,
//                                                                                               const QString& indexStr, quint32 number,
//                                                                                               QString& errMsg)
//{
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;

//    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList();

//    /* 判断DTE在线状态 */
//    auto deeOnlineResult = ZwzxRemoteControlCmdHelper::getOnlineDTE();
//    if (!deeOnlineResult)
//    {
//        errMsg = deeOnlineResult.msg();
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    auto deviceID = deeOnlineResult.value();

//    /* 根据编号获取主控中心 */
//    QString bs;
//    if (!TaskCenterStationNetMap::getMapValue(bs, indexStr, number))
//    {
//        errMsg = QString("未找到当前编号%1对应的任务中心").arg(number);
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    TaskCenterData* taskCenterData = nullptr;
//    TaskCenterMap taskCenterMap;
//    GlobalData::getTaskCenterInfo(taskCenterMap);
//    for (auto& item : taskCenterMap)
//    {
//        if (item.centerCode == bs)
//        {
//            taskCenterData = &item;
//            break;
//        }
//    }
//    if (taskCenterData == nullptr)
//    {
//        errMsg = QString("未找到当前编号%1对应的任务中心").arg(number);
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }
//    QList<TaskTimeEntity>* listEntity = nullptr;
//    if (taskTimeList.m_dataMap.contains(STTC))
//    {
//        listEntity = &taskTimeList.m_dataMap[STTC];
//    }
//    else if (taskTimeList.m_dataMap.contains(Skuo2))
//    {
//        listEntity = &taskTimeList.m_dataMap[Skuo2];
//    }
//    else if (taskTimeList.m_dataMap.contains(KaKuo2))
//    {
//        listEntity = &taskTimeList.m_dataMap[KaKuo2];
//    }
//    if (listEntity == nullptr)
//    {
//        errMsg = "当前模式不正确";
//        return ZwzxRemoteControlCmdReplyResult::Error;
//    }

//    /* 送数开关 */
//    QVariantMap tempMap;
//    tempMap["CenterID"] = taskCenterData->uacAddr;
//    singleCmdHelper.packSingleCommand("Step_DTE_SENDDATA_START", packCommand, deviceID);
//    Wait_Success();

//    tempMap.clear();
//    for (auto& item : *listEntity)
//    {
//        tempMap["Mid"] = item.m_taskBz;
//        tempMap["CenterID"] = taskCenterData->uacAddr;
//        singleCmdHelper.packSingleCommand("Step_DTE_CENTER_SENDDATA_START", packCommand, deviceID);
//        Wait_Success();
//    }

//    return ZwzxRemoteControlCmdReplyResult::Correct;
//}
