#include "ZeroCorrection.h"
#include "AutorunPolicyMessageSerialize.h"
#include "ControlFlowHandler.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "OnekeyXLHandler.h"
#include "PlanRunMessageDefine.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"
#include "UpLinkHandler.h"
#include <QDebug>
#include <QThread>

// int ZeroCorrection::typeId = qRegisterMetaType<ZeroCorrection>();
ZeroCorrection::ZeroCorrection(QObject* parent)
    : BaseEvent(parent)
{
}

void ZeroCorrection::doSomething()
{
    taskMachine->updateStatus("ZeroCorrection", TaskStepStatus::Running);
    // 加载校零需要的数据
    QString uuid = taskMachine->getMachineUUID();
    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);

    // 先判断是否需要进行校零
    if (!taskTimeList.m_is_zero)
    {
        notifyInfoLog(QString("当前任务配置为不校零，跳过校零流程"));
        taskMachine->updateStatus("ZeroCorrection", TaskStepStatus::Continue);
        notify("TurnToWaitingPoint");
        return;
    }
    // 判断有没有需要校零的任务，如果有需要的，就进行校零，没有就直接跳过，进入转等待点
    bool haveCkMode = false;
    auto msg = taskTimeList.manualMessage;
    auto linkMap = msg.linkLineMap;
    msg.linkLineMap.clear();
    for (auto iter = linkMap.begin(); iter != linkMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            msg.linkLineMap.insert(workMode, iter.value());
            haveCkMode = true;
        }
    }
    if (!haveCkMode)
    {
        notifyInfoLog(QString("当前任务不需要进行校零，自动跳过校零流程"));
        taskMachine->updateStatus("ZeroCorrection", TaskStepStatus::Continue);
        notify("TurnToWaitingPoint");
        return;
    }

    notifyInfoLog("校零处理流程开始执行");
    VoiceAlarmPublish::publish("校零开始");

    // ACU 拉偏天线，用于校零
    acu2Point(taskTimeList);

    ExitCheckVoid();
    //开始执行校零
    int xlResultCount = 0;
    for (auto& link : msg.linkLineMap)
    {
        OnekeyXLHandler onekeyXLHandler;
        connect(&onekeyXLHandler, &BaseHandler::signalSendToDevice, this, &ZeroCorrection::signalSendToDevice);
        connect(&onekeyXLHandler, &BaseHandler::signalInfoMsg, this, &ZeroCorrection::notifyInfoLog);        //提示日志
        connect(&onekeyXLHandler, &BaseHandler::signalWarningMsg, this, &ZeroCorrection::notifyWarningLog);  //警告日志
        connect(&onekeyXLHandler, &BaseHandler::signalErrorMsg, this, &ZeroCorrection::notifyErrorLog);      //错误日志
        connect(&onekeyXLHandler, &BaseHandler::signalSpecificTipsMsg, this, &ZeroCorrection::notifySpecificTipsLog);

        onekeyXLHandler.setRunningFlag(m_runningFlag);
        onekeyXLHandler.setManualMessage(msg);
        onekeyXLHandler.setLinkLine(link);
        xlResultCount += static_cast<int>(onekeyXLHandler.handle());
    }

    notifyInfoLog(QString("校零处理流程完成"));
    //QThread::sleep(1);
    if (xlResultCount == msg.linkLineMap.size())
    {
        VoiceAlarmPublish::publish("校零成功");
    }
    else
    {
        VoiceAlarmPublish::publish("校零失败");
    }
    taskMachine->updateStatus("ZeroCorrection", xlResultCount == msg.linkLineMap.size() ? TaskStepStatus::Finish : TaskStepStatus::Error);
    notify("TurnToWaitingPoint");
}

void ZeroCorrection::acu2Point(const TaskTimeList& taskTimeList)
{
    /* 获取天线配置参数 */
    /* 自动化运行策略数据获取 */
    AutorunPolicyData autorunPolicyData;
    GlobalData::getAutorunPolicyData(autorunPolicyData);
    auto offsetX = autorunPolicyData.policy.value("ZCaOffsetX", 5).toDouble();
    auto offsetY = autorunPolicyData.policy.value("ZCaOffsetY", 5).toDouble();
    //天线拉偏

    //获取当前角度
    // 当前方位角
    auto currentAZ = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentAZ");
    // 当前俯仰角
    auto currentPA = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentPA");
    if(!currentAZ.isValid() || !currentPA.isValid())
    {
        notifyInfoLog("获取当前ACU角度失败，跳过角度拉偏");
        return;
    }

    // 拉偏天线角度
    QMap<QString, QVariant> acuAngleMap;
    auto desAZ = currentAZ.toDouble() + offsetX;
    auto desPA = currentPA.toDouble() + offsetY;
    acuAngleMap["DesigAzimu"] = desAZ;   //指定方位角
    acuAngleMap["DesigElevat"] = desPA;  //指定俯仰角

    if(desAZ < 0.0 || desAZ > 360.0)
    {
        notifyInfoLog("水平拉偏角度超限跳过拉偏");
        return;
    }
    if(desPA < 0.0 || desPA > 90.0)
    {
        notifyInfoLog("俯仰拉偏角度超限跳过拉偏");
        return;
    }
    // 控制ACU拉偏角度到方位%1，俯仰%2
    m_singleCommandHelper.packSingleCommand("Step_ACU_To_Position", m_packCommand, taskTimeList.onlineACU, acuAngleMap);
    if (!waitExecSuccess(m_packCommand))
    {
        return;
    }

    QStringList keys;
    keys << "CurrentAZ"
         << "CurrentPA";

    int timeOut = GlobalData::getCmdTimeCount(40);
    for (auto i = 0; i < timeOut; ++i)
    {
        ExitCheckVoid();
        //实时获取ACU的值 当前方位俯仰
        auto currentAZPA = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, keys);
        if (fabs(desAZ - currentAZPA["CurrentAZ"].toDouble()) < 1 && fabs(desPA - currentAZPA["CurrentPA"].toDouble()) < 1)
        {
            notifyInfoLog(QString("控制ACU拉偏角度到方位%1，俯仰%2 成功").arg(desAZ).arg(desPA));
            return;
        }
        GlobalData::waitCmdTimeMS();
    }

    notifyErrorLog(QString("控制ACU拉偏角度到方位%1，俯仰%2 超时").arg(desAZ).arg(desPA));
}
