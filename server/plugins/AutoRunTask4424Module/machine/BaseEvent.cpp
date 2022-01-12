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
    deviceWorkTask = taskMachine->deviceWorkTask();
}

void BaseEvent::notify(const QString& nextStatus) { emit notifyMachine(nextStatus); }

void BaseEvent::notifyInfoLog(const QString& logStr)
{
    AutoTaskLogPublish::infoMsg(deviceWorkTask.m_uuid, deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::infoMsg(logStr);
}

void BaseEvent::notifyWarningLog(const QString& logStr)
{
    AutoTaskLogPublish::warningMsg(deviceWorkTask.m_uuid, deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::warningMsg(logStr);
}

void BaseEvent::notifyErrorLog(const QString& logStr)
{
    AutoTaskLogPublish::errorMsg(deviceWorkTask.m_uuid, deviceWorkTask.m_lord_with_target, logStr);
    SystemLogPublish::errorMsg(logStr);
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

    // 从循环读取响应的ID
    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        QThread::sleep(1);
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

bool BaseEvent::getCurrentStationUACAddr(qint32& ckAddr, QString& errMsg)
{
    StationResManagementMap resManagerMap;
    GlobalData::getStationResManagementInfo(resManagerMap);

    if (resManagerMap.isEmpty())
    {
        errMsg = "当前站资源数据为空";
        return false;
    }

    // UAC地址装订（测控基带）   从站资源获取
    QString uacAddrStr;          /* 测控 */
    QString dataTransUACAddrStr; /* 数传 */

    auto firstData = resManagerMap.first();
    uacAddrStr = firstData.ttcUACAddr;
    dataTransUACAddrStr = firstData.dataTransUACAddr;

    if (uacAddrStr.isEmpty())
    {
        errMsg = "UAC地址获取失败,请检查站资源管理是否有本站相关UAC地址";
        return false;
    }
    ckAddr = uacAddrStr.toInt(0, 16);
    return true;
}

Optional<DeviceID> BaseEvent::getOnlineACU()
{
    using ResType = Optional<DeviceID>;
    DeviceID acuADeviceID(ACU_A);
    DeviceID acuBDeviceID(ACU_B);

    return ResType(acuADeviceID);  // 暂时没有设备，这里直接返回，后面删除 20211028

    if (!GlobalData::getDeviceOnline(acuADeviceID) && !GlobalData::getDeviceOnline(acuBDeviceID))
    {
        return ResType(ErrorCode::DevOffline, "ACU主备设备离线，任务失败");
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
        return ResType(acuADeviceID);
    }
    else if (tempVarB.toInt() == 1)
    {
        return ResType(acuBDeviceID);
    }

    return ResType(ErrorCode::InvalidData, "未找到当前在线ACU，任务失败");
}

bool BaseEvent::dteOnlineCheck()
{
    // 20211021 当前两台DTE配置完全一致         （4424还未改，暂时也按照4426的使用）
    // 只有一个组播地址 一个发送端口 一个接收端口
    auto deviceID = m_singleCommandHelper.getSingleCmdDeviceID("Step_DTE_CTRLMODE", "0x8001");
    if (!deviceID.isValid())
    {
        return false;
    }
    return GlobalData::getDeviceOnline(deviceID);
}

void BaseEvent::setIsExit(bool isExit)
{
    m_isExit = isExit;
    signalExitRun(m_isExit);
}

void BaseEvent::startEvent() { doSomething(); }
