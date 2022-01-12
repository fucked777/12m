#include "ZeroCorrection.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "OnekeyXLHandler.h"
#include "RedisHelper.h"
#include "TaskGlobalInfo.h"
#include "TaskRunCommonHelper.h"
#include "ZeroCalibrationRoutor.h"

#include <QDebug>
#include <QThread>

// int ZeroCorrection::typeId = qRegisterMetaType<ZeroCorrection>();
ZeroCorrection::ZeroCorrection(QObject* parent)
    : BaseEvent(parent)
{
}

void ZeroCorrection::doSomething()
{
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

    //开始执行校零
    for (auto& link : msg.linkLineMap)
    {
        OnekeyXLHandler onekeyXLHandler;
        connect(&onekeyXLHandler, &BaseHandler::signalSendToDevice, this, &ZeroCorrection::signalSendToDevice);
        connect(&onekeyXLHandler, &BaseHandler::signalInfoMsg, this, &ZeroCorrection::notifyInfoLog);        //提示日志
        connect(&onekeyXLHandler, &BaseHandler::signalWarningMsg, this, &ZeroCorrection::notifyWarningLog);  //警告日志
        connect(&onekeyXLHandler, &BaseHandler::signalErrorMsg, this, &ZeroCorrection::notifyErrorLog);      //错误日志

        onekeyXLHandler.setManualMessage(msg);
        onekeyXLHandler.setLinkLine(link);
        onekeyXLHandler.setRestoreRWLink(true, Skuo2);
        onekeyXLHandler.handle();
    }

    notifyInfoLog(QString("校零处理流程完成"));
    QThread::sleep(1);
    notify("TurnToWaitingPoint");

    //    // 4413
    //    // 加载校零需要的数据
    //    QString uuid = taskMachine->getMachineUUID();
    //    auto taskTimeList = TaskGlobalInfoSingleton::getInstance().taskTimeList(uuid);
    //    auto m_ckDeviceMap = taskTimeList.m_ckDeviceMap;
    //    auto m_ckQdDeviceMap = taskTimeList.m_ckQdDeviceMap;
    //    auto m_hpaDeviceMap = taskTimeList.m_hpaDeviceMap;

    //    auto m_upFrequencyMap = taskTimeList.m_upFrequencyMap;
    //    auto m_downFrequencyMap = taskTimeList.m_downFrequencyMap;
    //    auto m_upFrequencyListMap = taskTimeList.m_upFrequencyListMap;
    //    auto m_downFrequencyListMap = taskTimeList.m_downFrequencyListMap;

    //    // 判断有没有需要校零的任务，如果有需要的，就进行校零，没有就直接跳过，进入转等待点
    //    bool haveCkMode = false;
    //    for (auto workMode : taskTimeList.m_dataMap.keys())
    //    {
    //        if (SystemWorkModeHelper::isMeasureContrlWorkMode((SystemWorkMode)workMode))
    //        {
    //            haveCkMode = true;
    //        }
    //    }
    //    if (!haveCkMode)
    //    {
    //        notifyInfoLog(QString("当前任务不需要进行校零，自动跳过校零流程"));
    //        taskMachine->updateStatus("ZeroCorrection", TaskStepStatus::Continue);
    //        notify("TurnToWaitingPoint");
    //        return;
    //    }

    //    notifyInfoLog("校零处理流程开始执行");
    //    VoiceAlarmPublish::publish("校零开始");

    //    if (haveCkMode)
    //    {
    //        // ACU 拉偏天线，用于校零
    //        notifyInfoLog("缺少ACU，跳过天线拉偏");
    //        // acu2Point(taskTimeList);

    //        //开始执行校零
    //        for (auto workMode : taskTimeList.m_dataMap.keys())
    //        {
    //            auto basezZeroCalibrationHandle = ZeroCalibrationRoutor::factory(SystemWorkMode(workMode));
    //            if (basezZeroCalibrationHandle == nullptr)
    //            {
    //                notifyErrorLog("校零错误,未获取到指定模式的校零方法");
    //                return;
    //            }
    //            connect(basezZeroCalibrationHandle, &BaseZeroCalibrationHandler::signalSendToDevice, this, &BaseEvent::signalSendToDevice);
    //            basezZeroCalibrationHandle->setTaskTimeList(taskTimeList);
    //            bool isZeroSuccess = basezZeroCalibrationHandle->handle();
    //            if (isZeroSuccess)
    //            {
    //                notifyInfoLog("校零成功");
    //            }
    //            else
    //            {
    //                taskMachine->updateStatus("ZeroCorrection", TaskStepStatus::Exception);
    //                notifyErrorLog("校零失败");
    //            }
    //            delete basezZeroCalibrationHandle;
    //            basezZeroCalibrationHandle = nullptr;
    //        }
    //    }
    //    notifyInfoLog(QString("校零处理流程完成"));
    //    QThread::sleep(1);
    //    notify("TurnToWaitingPoint");
}

void ZeroCorrection::acu2Point(const TaskTimeList& taskTimeList)
{
    //天线拉偏

    //获取当前角度
    // 当前方位角
    auto currentAZ = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentAZ");
    // 当前俯仰角
    auto currentPA = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentPA");
    // 拉偏天线角度
    QMap<QString, QVariant> paramMap;
    auto desAZ = currentAZ.toDouble() + taskTimeList.commonTaskConfig.zeroFW;
    auto desPA = currentPA.toDouble() + taskTimeList.commonTaskConfig.zeroFY;
    paramMap["DesigAzimu"] = desAZ;   // 指定方位角
    paramMap["DesigElevat"] = desPA;  // 指定俯仰角

    // 控制ACU拉偏角度到指定方位和俯仰角度
    m_singleCommandHelper.packSingleCommand("StepACU_MoveToPosition", m_packCommand, taskTimeList.onlineACU, paramMap);
    waitExecSuccess(m_packCommand);

    notifyInfoLog(QString("等待ACU拉偏到方位%1°，俯仰%2°").arg(desAZ).arg(desPA));
    for (auto i = 0; i < 40; ++i)
    {
        // 获取当前方位角
        currentAZ = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentAZ");
        // 获取当前俯仰角
        currentPA = GlobalData::getReportParamData(taskTimeList.onlineACU, 1, "CurrentPA");

        // 误差小于1表示拉偏到位
        if (fabs(desAZ - currentAZ.toDouble()) < 1 && fabs(desPA - currentPA.toDouble()) < 1)
        {
            notifyInfoLog(QString("控制ACU拉偏角度到方位%1°，俯仰%2°成功").arg(desAZ).arg(desPA));
            return;
        }
        QThread::msleep(500);
    }

    notifyErrorLog(QString("控制ACU拉偏角度到方位%1°，俯仰%2°超时").arg(desAZ).arg(desPA));

    //    // QThread::sleep(20);
    //    PackCommand packCommand;
    //    //天线拉偏移
    //    DeviceID deviceACUMaster;  //主用的ACU
    //    auto onlineACUResult = getOnlineACU();
    //    if (!onlineACUResult)
    //    {
    //        notifyErrorLog(onlineACUResult.msg());
    //        // notify("error");
    //        // return;
    //        onlineACUResult = Optional<DeviceID>(ACU_A); /* 有设备或者正常调试以后 删出 此行 */
    //    }
    //    deviceACUMaster = onlineACUResult.value();

    //    //获取当前角度
    //    //获取当前俯仰

    //    QString CurrentAZParamID = "CurrentAZ";  // 当前方位角
    //    QString CurrentPAParamID = "CurrentPA";  // 当前俯仰角
    //    QVariant CurrentAZ;                      // 当前方位角
    //    QVariant CurrentPA;                      // 当前俯仰角
    //    //        globalSystemData->getUnpackMessageData(deviceACUMaster, 0xFFFF, 1, CurrentAZParamID, CurrentAZ);
    //    //        globalSystemData->getUnpackMessageData(deviceACUMaster, 0xFFFF, 1, CurrentPAParamID, CurrentPA);
    //    // 拉偏天线角度
    //    QMap<QString, QVariant> acuAngleMap;
    //    acuAngleMap["DesigAzimu"] = CurrentAZ.toDouble() + taskTimeList.m_commonTaskConfig.m_zeroFW;   //指定方位角
    //    acuAngleMap["DesigElevat"] = CurrentPA.toDouble() + taskTimeList.m_commonTaskConfig.m_zeroFY;  //指定俯仰角

    //    // 控制ACU拉偏角度到方位%1，俯仰%2
    //    m_singleCommandHelper.packSingleCommand("StepACU_MoveToPosition", packCommand, deviceACUMaster, acuAngleMap);
    //    waitExecSuccess(packCommand);

    //    for (auto i = 0; i < 40; i++)
    //    {
    //        //实时获取ACU的值
    //        //            globalSystemData->getUnpackMessageData(deviceACUMaster, 0xFFFF, 1, CurrentAZParamID, CurrentAZ);
    //        //            globalSystemData->getUnpackMessageData(deviceACUMaster, 0xFFFF, 1, CurrentPAParamID, CurrentPA);
    //        if (fabs(acuAngleMap["DesigAzimu"].toDouble() - CurrentAZ.toDouble()) < 1 &&
    //            fabs(acuAngleMap["DesigElevat"].toDouble() - CurrentPA.toDouble()) < 1)
    //        {
    //            notifyInfoLog(
    //                QString("控制ACU拉偏角度到方位%1，俯仰%2
    //                成功").arg(acuAngleMap["DesigAzimu"].toDouble()).arg(acuAngleMap["DesigElevat"].toDouble()));
    //            break;
    //        }
    //        QThread::msleep(500);
    //    }
}
