#include "AutoRunTask4424Service.h"
#include "CppMicroServicesUtility.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "TaskPlanSql.h"
#include "TaskScheduling.h"

AutoRunTask4424Service::AutoRunTask4424Service(cppmicroservices::BundleContext /*context*/)
    : INetMsg(nullptr)
{
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData");
    qRegisterMetaType<CurrentRunningTaskPlanData>("const CurrentRunningTaskPlanData&");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData&");
    // 初始化线程
    if (nullptr == mTaskScheduling)
    {
        mTaskScheduling = new TaskScheduling();
        connect(mTaskScheduling, &TaskScheduling::signalSendToDevice, this, &AutoRunTask4424Service::sendByteArrayToDevice);

        mTaskScheduling->start();
    }

    // 默认自动化运行任务
    GlobalData::setAutoRunTaskFlag(true);

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskLogReadable, this, &AutoRunTask4424Service::slotReadAutoRunTaskLog);

    startTimer(1000);
}

AutoRunTask4424Service::~AutoRunTask4424Service()
{
    qWarning() << "任务自动化线程开始退出";
    if (mTaskScheduling)
    {
        mTaskScheduling->stopRing();
        mTaskScheduling->wait();
        delete mTaskScheduling;
        mTaskScheduling = nullptr;
    }
    qWarning() << "任务自动化线程完成退出";
}

void AutoRunTask4424Service::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    // 是否进行自动化运行任务
    mTaskScheduling->setIsAutoRun(GlobalData::getAutoRunTaskFlag());
}

void AutoRunTask4424Service::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    this->silenceSend(protocal);
}

void AutoRunTask4424Service::slotReadAutoRunTaskLog(const AutoTaskLogData& data)
{
    // 任务自动化运行日志入库
    TaskPlanSql::insertAutoTaskLog(data);
}
