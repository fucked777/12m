#include "BaseEvent.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "AutoRunTaskMessageDefine.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PackDefine.h"
#include "RedisHelper.h"
#include "StationResManagementDefine.h"
#include "TaskPlanMessageDefine.h"

#include <QApplication>
#include <QDebug>
#include <QMap>
#include <QThread>

BaseEvent::BaseEvent(QObject* parent)
    : QObject(parent)
{
    connect(this, &BaseEvent::start, this, &BaseEvent::startEvent);
}

BaseEvent::~BaseEvent() {}

void BaseEvent::doSomething() {}

void BaseEvent::dispatch(const QByteArray&) {}

void BaseEvent::registerMachine(TaskMachine* taskMachine)
{
    this->taskMachine = taskMachine;
    m_taskPlanData = taskMachine->taskPlan();
}

void BaseEvent::notify(const QString& nextStatus)
{
    m_statusChangeMutex.store(true);
    m_nextStatus = nextStatus;
    m_statusChangeMutex.store(false);
}
QString BaseEvent::nextStatus()
{
    return m_statusChangeMutex ? QString() : m_nextStatus;
}

void BaseEvent::notifyInfoLog(const QString& logStr)
{
    AutoTaskLogPublish::infoMsg(m_taskPlanData.deviceWorkTask.m_uuid, m_taskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::infoMsg(logStr);
}

void BaseEvent::notifyWarningLog(const QString& logStr)
{
    AutoTaskLogPublish::warningMsg(m_taskPlanData.deviceWorkTask.m_uuid, m_taskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::warningMsg(logStr);
}

void BaseEvent::notifyErrorLog(const QString& logStr)
{
    AutoTaskLogPublish::errorMsg(m_taskPlanData.deviceWorkTask.m_uuid, m_taskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::errorMsg(logStr);
}
void BaseEvent::notifySpecificTipsLog(const QString& logStr)
{
    AutoTaskLogPublish::specificTipsMsg(m_taskPlanData.deviceWorkTask.m_uuid, m_taskPlanData.deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::specificTipsMsg(logStr);
}

bool BaseEvent::waitExecSuccess(const PackCommand& packCommand, uint ttl)
{
    if (!packCommand.errorMsg.isEmpty())
    {
        notifyErrorLog(packCommand.errorMsg);
        return false;
    }

    if (packCommand.data.isEmpty())
    {
        notifyErrorLog(QString("系统组包失败，请检查配置，当前的设备ID：0x%1，模式Id：0x%2，命令Id：%3，命令类型：%4")
                           .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                           .arg(packCommand.cmdId)
                           .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType)));

        return false;
    }

    // 当前命令操作信息(如果为空，根据下发的参数生成操作信息，否则使用配置文件配置的操作信息)
    auto currentCmdOperatorInfo = (packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo);

    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1设备离线，发送%2命令%3失败")
                       .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
                            currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));

        notifyErrorLog(msg);
        return false;
    }

    // 发送数据
    emit signalSendToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    notifyInfoLog(currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl == 0)
    {
        return true;
    }
    ttl = GlobalData::getCmdTimeCount(ttl);
    // 从循环读取响应的ID
    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        if (isExit())
        {
            return false;
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
            QString message = QString("%1 (%2)").arg(currentCmdOperatorInfo).arg(cmdExecResultDesc);

            if (cmdResponce.responseResult == ControlCmdResponseType::Success)
            {
                notifyInfoLog(message);
            }
            else if (cmdResponce.responseResult == ControlCmdResponseType::SubControlRejected)
            {
                notifyWarningLog(message);
            }
            else
            {
                notifyErrorLog(message);
            }
            // 接收到响应就直接返回
            return true;
        }
    }
    // 超时未响应
    notifyErrorLog(QString("%1，超时未响应").arg(currentCmdOperatorInfo));
    qWarning() << QString("%1，超时未响应").arg(currentCmdOperatorInfo) << packCommand.data.toHex();

    return false;
}

void BaseEvent::setRunningFlag(std::atomic<bool>* runningFlag) { m_runningFlag = runningFlag; }
bool BaseEvent::isRunning()
{
    if (m_runningFlag == nullptr)
    {
        return true;
    }
    return m_runningFlag->load();
}
bool BaseEvent::isExit()
{
    if (m_runningFlag == nullptr)
    {
        return false;
    }
    return !m_runningFlag->load();
}

void BaseEvent::startEvent() { doSomething(); }
