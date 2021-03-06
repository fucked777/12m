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
        notifyErrorLog(QString("??????????????????????????????????????????????????????ID???0x%1?????????Id???0x%2?????????Id???%3??????????????????%4")
                           .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                           .arg(packCommand.cmdId)
                           .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType)));

        return false;
    }

    // ????????????????????????(??????????????????????????????????????????????????????????????????????????????????????????????????????)
    auto currentCmdOperatorInfo = (packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo);

    // ????????????????????????
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1?????????????????????%2??????%3??????")
                       .arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(),
                            currentCmdOperatorInfo.isEmpty() ? "" : QString("(%1)").arg(currentCmdOperatorInfo));

        notifyErrorLog(msg);
        return false;
    }

    // ????????????
    emit signalSendToDevice(packCommand.data);

    // ???????????????????????????????????????
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    notifyInfoLog(currentCmdOperatorInfo);

    // ??????????????????????????????
    if (ttl == 0)
    {
        return true;
    }
    ttl = GlobalData::getCmdTimeCount(ttl);
    // ????????????????????????ID
    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        if (isExit())
        {
            return false;
        }
        GlobalData::waitCmdTimeMS();
        --ttl;

        // ????????????????????????
        QString cmdResponceStr;
        if (!RedisHelper::getInstance().getData(redisFindKey, cmdResponceStr))
        {
            continue;
        }
        ControlCmdResponse cmdResponce;
        cmdResponceStr >> cmdResponce;

        // ????????????????????????????????????
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
            // ??????????????????????????????
            return true;
        }
    }
    // ???????????????
    notifyErrorLog(QString("%1??????????????????").arg(currentCmdOperatorInfo));
    qWarning() << QString("%1??????????????????").arg(currentCmdOperatorInfo) << packCommand.data.toHex();

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
