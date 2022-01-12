#include "AutomateTestBase.h"
#include "AutomateTest.h"
#include "AutomateTestHelper.h"
#include "AutomateTestSerialize.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "ConfigMacroMessageSerialize.h"
#include "ControlFlowHandler.h"
#include "DeviceProcessMessageSerialize.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "SingleCommandHelper.h"
#include "Utility.h"
#include <QApplication>
#include <QMap>
#include <QVariant>
#include <atomic>

class AutomateTestBaseImpl
{
public:
    AutomateTest* parent;
    ProtocolPack pack;                       /* 协议包 */
    CmdResult curResult;                     /* 命令结果 */
    QList<CmdResult> curResultList;          /* 命令结果 */
    ControlCmdResponse curResponce;          /* 命令响应 */
    bool isRecvResult{ false };              /* 成功接收命令结果 */
    bool isRecvResponce{ false };            /* 成功接收命令响应 */
    PackCommand m_currentPackCmd;            /* 用于上报结果对比 */
    std::atomic<bool> isCallStop{ false };   /* 是否调用停止函数 */
    std::atomic<bool> runningFlag;           /* 退出标志 */
    AutoTestSpin spin{ AutoTestSpin::Left }; /* 自动化测试的当前旋向 */
    ControlFlowHandler controlFlowHandler;

    AutomateTestBaseImpl(AutomateTest* parent_)
        : parent(parent_)
    {
    }
};

AutomateTestBase::AutomateTestBase(AutomateTest* parent)
    : QThread(nullptr)
    , m_singleCommandHelper(new SingleCommandHelper)
    , m_impl(new AutomateTestBaseImpl(parent))
{
    connect(m_impl->parent, &AutomateTest::sg_deviceCMDResult, this, &AutomateTestBase::acceptDeviceCMDResultMessage);
    connect(m_impl->parent, &AutomateTest::sg_deviceCMDResponce, this, &AutomateTestBase::acceptDeviceCMDResponceMessage);
}
AutomateTestBase::~AutomateTestBase() {}

void AutomateTestBase::startTest(const ProtocolPack& pack)
{
    m_impl->pack = pack;
    m_impl->pack.data >> m_testPlanItem;
    start();
}
void AutomateTestBase::stopTest()
{
    m_impl->runningFlag = false;
    quit();
    wait();

    /* 善后处理 */
    if (m_impl->isCallStop)
    {
        endOfTest();
        m_impl->isCallStop = false;
    }
}

void AutomateTestBase::signalInfoMsg(const QString& msg) { pushLog(msg, false); }

void AutomateTestBase::signalWarningMsg(const QString& msg) { pushLog(msg, false); }

void AutomateTestBase::signalErrorMsg(const QString& msg) { pushLog(msg, false); }

void AutomateTestBase::signalSendToDevice(const QByteArray& data) { sendByteArrayToDevice(data); }
void AutomateTestBase::run()
{
    m_impl->isCallStop = true;
    m_impl->runningFlag = true;
    pushLog(QString("%1 开始").arg(m_testPlanItem.testTypeName));
    if (!pretreatment()) /* 参数预处理 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    pushStatus(AutomateTestStatus::LinkConfiguration);
    if (!linkConfiguration()) /* 设置环路 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    pushStatus(AutomateTestStatus::ParameterSetting);
    if (!parameterSetting()) /* 参数设置 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    pushStatus(AutomateTestStatus::StartTesting);
    pushStatus(AutomateTestStatus::Testing);
    CmdResult cmdResult;
    if (!testing(cmdResult)) /* 开始测试 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    if (!resultOperation(cmdResult)) /* 结果操作 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    endOfTest(); /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */

    pushStatus(AutomateTestStatus::EndOfTest);

    m_impl->isCallStop = false;
    m_impl->runningFlag = false;
    emit sg_end();
}
bool AutomateTestBase::pretreatment()
{
    /* 这里预先处理一下数据
     * 因为有的数据在所有测试中都可能用到
     */

    /* 配置宏,旋向 */
    auto scResult = AutomateTestHelper::getSpinAndConfigMacroData(m_testPlanItem.taskCode, m_testPlanItem.workMode, m_testPlanItem.dotDrequency);
    if (!scResult)
    {
        pushLog(scResult.msg(), true);
        return false;
    }
    m_impl->spin = std::get<0>(scResult.value());
    m_configMacroData = std::get<1>(scResult.value());

    QVariant up;
    QVariant down;
    MacroCommon::getFrequency(m_testPlanItem.taskCode, m_testPlanItem.workMode, m_testPlanItem.dotDrequency, up, down);
    m_upFreq = up.toDouble();
    m_downFreq = down.toDouble();

    if (qFuzzyIsNull(m_downFreq))
    {
        pushLog("测试失败:参数宏数据解析失败", true);
        return false;
    }

    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_testPlanItem.workMode) == SystemBandMode::SBand ||  //
        m_testPlanItem.workMode == KaKuo2 ||                                                                   //
        m_testPlanItem.workMode == KaDS)
    {
        if (qFuzzyIsNull(m_upFreq))
        {
            pushLog("测试失败:参数宏数据解析失败", true);
            return false;
        }
    }
    return true;
}
void AutomateTestBase::endOfTest() {}
bool AutomateTestBase::isRunning() const { return m_impl->runningFlag.load(); }
bool AutomateTestBase::isExit() const { return !m_impl->runningFlag.load(); }

void AutomateTestBase::pushStatus(AutomateTestStatus status)
{
    TestProcessACK statusACK;
    statusACK.status = status;
    m_impl->pack.data.clear();
    m_impl->pack.data << statusACK;
    m_impl->parent->silenceSendACK(m_impl->pack);
}

void AutomateTestBase::pushLog(const QString& msg, bool error)
{
    RedisHelper::getInstance().publish("AutomateTest", msg);
    /* 日志记录 */
    if (error)
    {
        /* 停止当前的标定 */
        TestProcessACK statusACK;
        statusACK.msg = msg;
        statusACK.status = AutomateTestStatus::Failed;

        m_impl->pack.data.clear();
        m_impl->pack.data << statusACK;
        m_impl->parent->silenceSendACK(m_impl->pack);
    }
}
void AutomateTestBase::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    m_impl->parent->silenceSend(protocal);
}
void AutomateTestBase::acceptDeviceCMDResultMessage(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data >> m_impl->curResult;
    if (m_impl->m_currentPackCmd.deviceID.sysID == m_impl->curResult.m_systemNumb &&
        m_impl->m_currentPackCmd.deviceID.devID == m_impl->curResult.m_deviceNumb &&
        m_impl->m_currentPackCmd.deviceID.extenID == m_impl->curResult.m_extenNumb && m_impl->m_currentPackCmd.cmdId == m_impl->curResult.m_cmdID)
    {
        //连续上报的结果
        m_impl->curResultList.append(m_impl->curResult);
        m_impl->isRecvResult = true;
    }
}
void AutomateTestBase::acceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data >> m_impl->curResponce;
    if (m_impl->m_currentPackCmd.deviceID == m_impl->curResponce.deviceID && m_impl->m_currentPackCmd.cmdId == m_impl->curResponce.cmdId)
    {
        m_impl->isRecvResponce = true;
    }
}

AutomateTestBase::ExecStatus AutomateTestBase::waitExecSuccessImpl(const PackCommand& packCommand, qint32 ttl, bool timeoutError)
{
    if (isExit())
    {
        return ExecStatus::Exit;
    }
    if (!packCommand.errorMsg.isEmpty())
    {
        pushLog(packCommand.errorMsg, true);
        return ExecStatus::Failed;
    }

    if (packCommand.data.isEmpty())
    {
        auto msg = QString("系统组包失败,请检查配置,当前的设备ID：0x%1,模式Id：0x%2,命令Id：%3,命令类型：%4")
                       .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                       .arg(packCommand.cmdId)
                       .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType));
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    // 当前命令操作信息
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1设备离线,发送%2命令(%3)失败").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    m_impl->isRecvResponce = false;
    m_impl->m_currentPackCmd = packCommand;

    // 发送数据
    sendByteArrayToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
    pushLog(currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl <= 0)
    {
        return ExecStatus::Success;
    }

    while (ttl > 0)
    {
        if (isExit())
        {
            return ExecStatus::Exit;
        }
        QThread::sleep(1);
        --ttl;

        /* 判断是否接收响应成功 */
        if (!m_impl->isRecvResponce)
        {
            continue;
        }

        auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(m_impl->curResponce.responseResult);
        auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);

        /* 判断是否执行成功 */
        if (ControlCmdResponseType::Success == m_impl->curResponce.responseResult)
        {
            pushLog(message);
            return ExecStatus::Success;
        }

        pushLog(message, true);
        return ExecStatus::Failed;
    }

    // 超时未响应
    if (timeoutError)
    {
        pushLog(QString("%1，超时未响应").arg(currentCmdOperatorInfo), true);
    }
    return ExecStatus::Timeout;
}

AutomateTestBase::ExecStatus AutomateTestBase::waitExecSuccess(const PackCommand& packCommand, qint32 ttl)
{
    return waitExecSuccessImpl(packCommand, ttl, true);
}
AutomateTestBase::ExecStatus AutomateTestBase::waitSingleStatusChange(const PackCommand& packCommand, const DeviceID& devID, int unitID,
                                                                      const QString& paramID, const QVariant& value, qint32 ttl)
{
    auto execStatus = waitExecSuccessImpl(packCommand, ttl, true);
    if (execStatus != ExecStatus::Success)
    {
        return execStatus;
    }

    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
    pushLog(QString("%1  等待状态切换中").arg(currentCmdOperatorInfo));

    /* 等待状态改变 */
    while (ttl > 0)
    {
        if (isExit())
        {
            return ExecStatus::Exit;
        }
        QThread::sleep(1);
        QCoreApplication::processEvents();
        --ttl;

        auto statusValue = GlobalData::getReportParamData(devID, unitID, paramID);
        if (statusValue.isValid() && statusValue == value)
        {
            auto message = QString("%1  等待状态切换成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            return ExecStatus::Success;
        }
    }

    auto message = QString("%1 等待状态切换超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}

AutomateTestBase::ExecStatus AutomateTestBase::waitExecResponceData(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count, qint32 ttl)
{
    m_impl->m_currentPackCmd = packCommand;
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    for (int i = 0; i < count; ++i)
    {
        m_impl->isRecvResult = false;   /* 成功接收命令结果 */
        m_impl->isRecvResponce = false; /* 成功接收命令响应 */

        /* 等待命令响应 */
        auto execStatus = waitExecSuccessImpl(packCommand, ttl, false);
        if (execStatus == ExecStatus::Timeout)
        {
            pushLog(QString("%1  未收到响应超时重试").arg(currentCmdOperatorInfo));
            continue;
        }
        else if (execStatus != ExecStatus::Success)
        {
            return execStatus;
        }

        pushLog(QString("%1  等待结果上报中").arg(currentCmdOperatorInfo));

        auto timeout = ttl;
        /* 等待结果 */
        while (timeout > 0)
        {
            if (isExit())
            {
                return ExecStatus::Exit;
            }
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --timeout;

            /* 判断是否接收结果成功 */
            if (!m_impl->isRecvResult)
            {
                continue;
            }

            auto message = QString("%1  结果上报成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            cmdResult = m_impl->curResult;
            return ExecStatus::Success;
        }
        pushLog(QString("%1  未收到结果超时重试").arg(currentCmdOperatorInfo));
    }

    auto message = QString("%1 等待结果上报超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}

AutomateTestBase::ExecStatus AutomateTestBase::waitExecResponceCondition(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count,
                                                                         qint32 ttl, const QVariant& arg, ConditionFunc func)
{
    m_impl->m_currentPackCmd = packCommand;
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    for (int i = 0; i < count; ++i)
    {
        m_impl->isRecvResult = false;   /* 成功接收命令结果 */
        m_impl->isRecvResponce = false; /* 成功接收命令响应 */

        /* 等待命令响应 */
        auto execStatus = waitExecSuccessImpl(packCommand, ttl, false);
        if (execStatus == ExecStatus::Timeout)
        {
            pushLog(QString("%1  未收到响应超时重试").arg(currentCmdOperatorInfo));
            continue;
        }
        else if (execStatus != ExecStatus::Success)
        {
            return execStatus;
        }

        pushLog(QString("%1  等待结果上报中").arg(currentCmdOperatorInfo));

        auto timeout = ttl;
        /* 等待结果 */
        while (timeout > 0)
        {
            if (isExit())
            {
                return ExecStatus::Exit;
            }
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --timeout;

            /* 判断是否接收结果成功 */
            if (!m_impl->isRecvResult)
            {
                continue;
            }
            QString errMsg;
            auto result = func(m_impl->curResult, errMsg, arg);
            if (result == ConditionResult::Error)
            {
                pushLog(QString("%1  结果数据不正确").arg(currentCmdOperatorInfo), true);
                return ExecStatus::Failed;
            }
            else if (result == ConditionResult::Wait)
            {
                m_impl->isRecvResult = false;
                continue;
            }

            auto message = QString("%1 结果上报成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            cmdResult = m_impl->curResult;
            return ExecStatus::Success;
        }
        pushLog(QString("%1: 未收到结果超时重试").arg(currentCmdOperatorInfo));
    }

    auto message = QString("%1 等待结果上报超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}

AutomateTestBase::ExecStatus AutomateTestBase::waitExecResponceMultiData(QList<CmdResult>& cmdResultList, const PackCommand& packCommand,
                                                                         qint32 count, qint32 ttl)
{
    m_impl->curResultList.clear();
    m_impl->m_currentPackCmd = packCommand;
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    for (int i = 0; i < count; ++i)
    {
        m_impl->isRecvResult = false;   /* 成功接收命令结果 */
        m_impl->isRecvResponce = false; /* 成功接收命令响应 */

        /* 等待命令响应 */
        auto execStatus = waitExecSuccessImpl(packCommand, ttl, false);
        if (execStatus == ExecStatus::Timeout)
        {
            pushLog(QString("%1  未收到响应超时重试").arg(currentCmdOperatorInfo));
            continue;
        }
        else if (execStatus != ExecStatus::Success)
        {
            return execStatus;
        }

        pushLog(QString("%1  等待结果上报中").arg(currentCmdOperatorInfo));

        bool isAcceptResult = false;
        auto timeout = ttl;
        /* 等待结果 */
        while (timeout > 0)
        {
            if (isExit())
            {
                return ExecStatus::Exit;
            }
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --timeout;

            /* 判断是否接收结果成功 */
            if (!m_impl->isRecvResult)
            {
                continue;
            }
            isAcceptResult = true;
        }
        if (isAcceptResult)
        {
            cmdResultList = m_impl->curResultList;
            auto message = QString("%1  结果上报成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            return ExecStatus::Success;
        }
        else
        {
            pushLog(QString("%1  未收到结果超时重试").arg(currentCmdOperatorInfo));
        }
    }

    auto message = QString("%1  等待结果上报超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}

void AutomateTestBase::notWaitExecSuccess(const PackCommand& packCommand) { waitExecSuccessImpl(packCommand, 0, true); }
void AutomateTestBase::notWaitExecSuccessBeQuiet(const PackCommand& packCommand) { waitExecSuccessImpl(packCommand, 0, false); }

bool AutomateTestBase::isLeftSpin() const { return m_impl->spin == AutoTestSpin::Left; }
bool AutomateTestBase::isRightSpin() const { return m_impl->spin == AutoTestSpin::Right; }

const AutomateTestPlanItem& AutomateTestBase::testPlanItem() const { return m_testPlanItem; }
const ConfigMacroData& AutomateTestBase::configMacroData() const { return m_configMacroData; }
double AutomateTestBase::upFreq() const { return m_upFreq; }
double AutomateTestBase::downFreq() const { return m_downFreq; }
bool AutomateTestBase::setConfigMacroAndParamMacro(const LinkType& linkType)
{
    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    msg.isManualContrl = true;
    msg.configMacroData = m_configMacroData;

    LinkLine linkLine;
    auto index = 1;
    linkLine.workMode = m_testPlanItem.workMode;
    linkLine.masterTargetNo = index;
    TargetInfo targetInfo;
    targetInfo.targetNo = index;
    targetInfo.workMode = m_testPlanItem.workMode;
    targetInfo.taskCode = m_testPlanItem.taskCode;
    targetInfo.pointFreqNo = m_testPlanItem.dotDrequency;
    linkLine.targetMap[index] = targetInfo;
    linkLine.linkType = linkType;
    msg.appendLine(linkLine);

    bool result = true;
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalSendToDevice, this, &AutomateTestBase::sendByteArrayToDevice);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalErrorMsg, [&](const QString& msg) {
        this->pushLog(msg, true);
        result = false;
        emit sg_end();
    });
    m_impl->controlFlowHandler.setRunningFlag(&(m_impl->runningFlag));
    m_impl->controlFlowHandler.handle(msg);
    return result;
}
