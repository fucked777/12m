#include "PhaseCalibrationService.h"
#include "AutoRunTaskMessageDefine.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "ControlFlowHandler.h"
#include "DeviceProcessMessageSerialize.h"
#include "EquipmentCombinationHelper.h"
#include "ExtendedConfig.h"
#include "OnekeyXXHandler.h"
#include "PhaseCalibration.h"
#include "PhaseCalibrationSerialize.h"
#include "PlanRunMessageDefine.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "SingleCommandHelper.h"
#include "SqlPhaseCalibration.h"
#include <QApplication>
#include <QMap>
#include <QVariant>

class AGCCalibrationServerAutoStop
{
public:
    AGCCalibrationServerAutoStop(PhaseCalibrationService* self_)
        : self(self_)
    {
    }
    ~AGCCalibrationServerAutoStop() { emit self->sg_end(); }

private:
    PhaseCalibrationService* self;
};

class PhaseCalibrationServiceImpl
{
public:
    PhaseCalibration* parent;
    PhaseCalibrationService* self;
    ProtocolPack pack;          /* 协议包 */
    StartPhaseCalibration item; /* 校相的参数 */
    QVariantMap replaceMap;     /* 替换参数 */
    SingleCommandHelper singleCmdHelper;
    std::atomic<bool> isRunningFlag{ false };

    PhaseCalibrationServiceImpl(PhaseCalibration* parent_, PhaseCalibrationService* self_)
        : parent(parent_)
        , self(self_)
    {
    }

    static Optional<int> getEmissPolar(ACUPhaseCalibrationFreqBand fb, const StartPhaseCalibration& item,
                                       SatelliteManagementData& satelliteManagementData);
    static Optional<quint64> createDevNumber(SystemWorkMode workMode);
};

Optional<int> PhaseCalibrationServiceImpl::getEmissPolar(ACUPhaseCalibrationFreqBand fb, const StartPhaseCalibration& item,
                                                         SatelliteManagementData& satelliteManagementData)
{
    using ResType = Optional<int>;
    auto& saveParameter = item.saveParameter;

    if (saveParameter.contains(fb))
    {
        auto tempValue = saveParameter.value(ACUPhaseCalibrationFreqBand::S);
        SystemOrientation orientation{ SystemOrientation::Unkonwn };
        auto result = satelliteManagementData.getSReceivPolar(tempValue.systemWorkMode, tempValue.dotDrequency, orientation);
        if (!result)
        {
            return ResType(ErrorCode::NotFound,
                           QString("未能获取%1模式的极化方式").arg(SystemWorkModeHelper::systemWorkModeToDesc(tempValue.systemWorkMode)));
        }

        return ResType(orientation == SystemOrientation::RCircumflex ? 2 : 1);
    }

    return ResType(1);
}

Optional<quint64> PhaseCalibrationServiceImpl::createDevNumber(SystemWorkMode workMode)
{
    using ResType = Optional<quint64>;
    /* 生成设备组合号 */
    /* 1.获取当前配置宏数据 */
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        auto message = "查找当前模式对应的配置宏数据失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 2.获取当前测控基带 */
    DeviceID ckDeviceID;
    if (!MacroCommon::getMasterCKJDDeviceID(workMode, ckDeviceID))
    {
        auto message = "获取主用测控基带失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 3.获取当前跟踪基带 */
    DeviceID gzDeviceID;
    if (!MacroCommon::getGZQDDeviceID(workMode, gzDeviceID))
    {
        auto message = "获取主用跟踪基带失败";
        return ResType(ErrorCode::NotFound, message);
    }
    gzDeviceID.sysID = 0x04;
    gzDeviceID.devID = 0x02;

    quint64 equipComb = 0;
    if (!EquipmentCombinationHelper::getCKDevNumber(workMode, configMacroData, ckDeviceID, gzDeviceID, equipComb))
    {
        auto message = "获取当前设备组合号失败";
        return ResType(ErrorCode::NotSupported, message);
    }
    return ResType(equipComb);
}
PhaseCalibrationService::PhaseCalibrationService(PhaseCalibration* parent)
    : QThread(parent)
    , m_impl(new PhaseCalibrationServiceImpl(parent, this))
{
}
PhaseCalibrationService::~PhaseCalibrationService() { delete m_impl; }

void PhaseCalibrationService::run()
{
    /* 退出函数后自动发送结束信号 */
    AGCCalibrationServerAutoStop autoStop(this);
    m_impl->isRunningFlag = true;

    /* 1.数据检查 */
    {
        auto& parameter = m_impl->item.parameter;
        auto& saveParameter = m_impl->item.saveParameter;

        if (parameter.freqBand == ACUPhaseCalibrationFreqBand::S ||     //
            parameter.freqBand == ACUPhaseCalibrationFreqBand::KaYC ||  //
            parameter.freqBand == ACUPhaseCalibrationFreqBand::KaSC)
        {
            if (saveParameter.size() != 1)
            {
                pushLog("当前参数错误:单模式校相但是保存结果数据数量超过预期", true);
                return;
            }
        }
        else if (parameter.freqBand == ACUPhaseCalibrationFreqBand::SKaYC)
        {
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::S))
            {
                pushLog("当前校相频段为S+Ka遥测,但是未能查找到S的数据", true);
                return;
            }
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::KaYC))
            {
                pushLog("当前校相频段为S+Ka遥测,但是未能查找到Ka遥测的数据", true);
                return;
            }
        }
        else if (parameter.freqBand == ACUPhaseCalibrationFreqBand::SKaSC)
        {
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::S))
            {
                pushLog("当前校相频段为S+Ka遥测,但是未能查找到S的数据", true);
                return;
            }
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::KaSC))
            {
                pushLog("当前校相频段为S+Ka遥测,但是未能查找到Ka数传的数据", true);
                return;
            }
        }
        else if (parameter.freqBand == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
        {
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::S))
            {
                pushLog("当前校相频段为S+Ka遥测+Ka数传,但是未能查找到S的数据", true);
                return;
            }
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::KaYC))
            {
                pushLog("当前校相频段为S+Ka遥测+Ka数传,但是未能查找到Ka遥测的数据", true);
                return;
            }
            if (!saveParameter.contains(ACUPhaseCalibrationFreqBand::KaSC))
            {
                pushLog("当前校相频段为S+Ka遥测+Ka数传,但是未能查找到Ka数传的数据", true);
                return;
            }
        }
    }

    /* 2.获取当前的主用ACU */
    DeviceID mainACU;
    {
        //获取基带的在线状态和本控状态
        DeviceID acuADeviceID(ACU_A);
        DeviceID acuBDeviceID(ACU_B);

        if (!GlobalData::getDeviceOnline(acuADeviceID) && GlobalData::getDeviceOnline(acuADeviceID))
        {
            pushLog("检测到当前ACU都离线,校相终止", true);
            return;
        }

        /*
         * 1 在线 表示当前主机
         * 2 离线 表示当前备机
         * 根据4413的情况ACU的主备是绝对的主备 不能两个同时使用的
         */
        auto tempVarA = GlobalData::getReportParamData(acuADeviceID, 0x01, "OnLineMachine");
        auto tempVarB = GlobalData::getReportParamData(acuBDeviceID, 0x01, "OnLineMachine");
        if (tempVarA.toInt() == 1)
        {
            mainACU = acuADeviceID;
        }
        else if (tempVarB.toInt() == 1)
        {
            mainACU = acuBDeviceID;
        }
        else
        {
            pushLog("未能找到当前主用ACU,校相终止", true);
            return;
        }
        if (!GlobalData::getDeviceSelfControl(mainACU))
        {
            pushLog("当前主用ACU分控,校相终止", true);
            return;
        }
    }

    /* 3.打链路和下宏
     * 这里要注意 不同于AGC标校 只管一个模式的链路就OK
     * 校相最多可能涉及到3种模式
     * 需要将所有涉及到的链路都打通
     */
    ManualMessage manualMessage;
    {
        manualMessage.manualType = ManualType::ConfigMacroAndParamMacro;
        manualMessage.ltDTETask = false;
        manualMessage.isTest = true;
        for (auto& item : m_impl->item.saveParameter)
        {
            auto index = 1;
            LinkLine linkLine;
            linkLine.workMode = item.systemWorkMode;
            linkLine.masterTargetNo = index;
            TargetInfo targetInfo;
            targetInfo.targetNo = index;
            targetInfo.workMode = item.systemWorkMode;
            targetInfo.taskCode = m_impl->item.parameter.taskCode;
            targetInfo.pointFreqNo = item.dotDrequency;
            linkLine.targetMap[item.dotDrequency] = targetInfo;
            linkLine.linkType = LinkType::RWFS;
            linkLine.masterTargetNo = item.dotDrequency;

            manualMessage.appendLine(linkLine);
            manualMessage.masterTaskCode = m_impl->item.parameter.taskCode;
        }

        ControlFlowHandler controlFlowHandler;
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &PhaseCalibrationService::sendByteArrayToDevice);
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, [&](const QString& msg) {
            this->pushLog(msg, true);
            m_impl->isRunningFlag = false;
        });
        controlFlowHandler.setRunningFlag(&(m_impl->isRunningFlag));
        controlFlowHandler.handle(manualMessage);
        if (!m_impl->isRunningFlag)
        {
            return;
        }
        manualMessage = controlFlowHandler.getNewManualMsg();
    }

    OnekeyXXHandler onekeyXXHandler;
    connect(&onekeyXXHandler, &OnekeyXXHandler::signalSendToDevice, this, &PhaseCalibrationService::sendByteArrayToDevice);
    connect(&onekeyXXHandler, &OnekeyXXHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&onekeyXXHandler, &OnekeyXXHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&onekeyXXHandler, &OnekeyXXHandler::signalSpecificTipsMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&onekeyXXHandler, &OnekeyXXHandler::signalErrorMsg, [&](const QString& msg) {
        this->pushLog(msg, true);
        m_impl->isRunningFlag = false;
    });

    onekeyXXHandler.setRunningFlag(&(m_impl->isRunningFlag));
    onekeyXXHandler.setManualMessage(manualMessage);

    auto isSuccess = onekeyXXHandler.handle();
    if (isSuccess)
    {
        pushLog("校相成功");
    }
    else
    {
        pushLog("校相失败");
    }
}

void PhaseCalibrationService::startServer(const ProtocolPack& pack)
{
    m_impl->pack = pack;
    m_impl->pack.data >> m_impl->item;
    start();
}
void PhaseCalibrationService::stopServer()
{
    disconnect();
    m_impl->isRunningFlag = false;

    PackCommand packCommand;
    m_impl->replaceMap["PhaseCtrl"] = 2;
    m_impl->singleCmdHelper.packSingleCommand("Step_ACU_QUICK_CALIBRATION_START", packCommand, m_impl->replaceMap);
    waitExecSuccess(packCommand, 0);
}

void PhaseCalibrationService::pushLog(const QString& msg, bool error)
{
    RedisHelper::getInstance().publish("PhaseCalibrationLog", msg);
    /* 日志记录 */
    if (error)
    {
        /* 停止当前的标定 */
        m_impl->pack.data.clear();
        m_impl->pack.data << OptionalNotValue(ErrorCode::TestError, msg);
        m_impl->parent->silenceSendACK(m_impl->pack);
    }
}

void PhaseCalibrationService::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    m_impl->parent->silenceSend(protocal);
}

PhaseCalibrationService::ExecStatus PhaseCalibrationService::waitExecSuccess(const PackCommand& packCommand, qint32 ttl)
{
    if (!packCommand.errorMsg.isEmpty())
    {
        pushLog(packCommand.errorMsg, true);
        return ExecStatus::Failed;
    }

    if (packCommand.data.isEmpty())
    {
        auto msg = QString("系统组包失败，请检查配置，当前的设备ID：0x%1，模式Id：0x%2，命令Id：%3，命令类型：%4")
                       .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                       .arg(packCommand.cmdId)
                       .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType));
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    // 当前命令操作信息(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
    auto currentCmdOperatorInfo = (packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1设备离线，发送%2命令%3失败").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    // 先清除当前的数据
    RedisHelper::getInstance().setData(redisFindKey, "");
    // 发送数据
    sendByteArrayToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl <= 0)
    {
        return ExecStatus::Success;
    }

    // 从循环读取响应的ID
    ttl = GlobalData::getCmdTimeCount(ttl);
    while (ttl > 0)
    {
        if (!m_impl->isRunningFlag)
        {
            return ExecStatus::Exit;
        }
        GlobalData::waitCmdTimeMS();
        --ttl;

        // 获取命令执行结果
        QString cmdResponceStr;
        if (!RedisHelper::getInstance().getData(redisFindKey, cmdResponceStr))
        {
            continue;
        }

        ControlCmdResponse cmdResponce;
        cmdResponceStr >> cmdResponce;

        // 如果是当前发送的命令响应
        if (cmdResponce.isValid() && cmdResponce.deviceID == packCommand.deviceID && cmdResponce.cmdId == packCommand.cmdId &&
            cmdResponce.cmdType == packCommand.cmdType)
        {
            auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
            auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);

            if (cmdResponce.responseResult == ControlCmdResponseType::Success)
            {
                pushLog(message);
                return ExecStatus::Success;
            }

            pushLog(message, true);
            return ExecStatus::Failed;
        }
    }

    // 超时未响应
    pushLog(QString("%1,超时未响应").arg(currentCmdOperatorInfo), true);
    return ExecStatus::Timeout;
}
