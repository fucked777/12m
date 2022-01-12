#include "AutoRunTask4426Service.h"
#include "CppMicroServicesUtility.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "CConverter.h"
#include "CommunicationSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlanRunMessageSerialize.h"
#include "StationNetCenterCmd.h"
#include "SubscriberHelper.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskPlanSql.h"
#include "TaskScheduling.h"
#include "HeartbeatMessageSerialize.h"

AutoRunTask4426Service::AutoRunTask4426Service(cppmicroservices::BundleContext /*context*/)
    : INetMsg(nullptr)
    , mTaskScheduling(new TaskScheduling)
    , mStationNetCenter(new StationNetCenterCmd)
{
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData");
    qRegisterMetaType<CurrentRunningTaskPlanData>("const CurrentRunningTaskPlanData&");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData&");

    registerSubscribe("AcceptDeviceMessage", &AutoRunTask4426Service::recvStationNetCenter, this);
    registerSubscribe("recvDeviceWorkTaskList", &AutoRunTask4426Service::recvDeviceWorkTaskList, this);
    // 初始化线程
    connect(mTaskScheduling, &TaskScheduling::signalSendToDevice, this, &AutoRunTask4426Service::sendByteArrayToDevice);
    connect(mTaskScheduling, &TaskScheduling::sg_taskResourceRelease, this, &AutoRunTask4426Service::taskResourceRelease);

    // 初始化处理站网中心命令线程

    connect(mStationNetCenter, &StationNetCenterCmd::signalSendToDevice, this, &AutoRunTask4426Service::sendByteArrayToDevice);
    connect(mStationNetCenter, &StationNetCenterCmd::signalSend2StationNetCenter, [this](const QByteArray& data) {
        /* 中心的ID这里设置为FE01 */
        static constexpr quint16 centerID = 0xFE01;
        auto sendData = CConverter::toByteArray<quint16>(centerID);
        sendData.append(data);

        ProtocolPack protocal;
        protocal.srcID = "";
        protocal.desID = "NetWorkManager";
        protocal.module = true;
        protocal.operation = "sendDataToOtherDevice";
        protocal.data = sendData;
        this->silenceSend(protocal);
    });

    // 默认自动化运行任务
    GlobalData::setAutoRunTaskFlag(true);

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskLogReadable, this, &AutoRunTask4426Service::slotReadAutoRunTaskLog);
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::sg_masterSlave,this,&AutoRunTask4426Service::masterSlave);
}

AutoRunTask4426Service::~AutoRunTask4426Service()
{
    if (mTaskScheduling)
    {
        mTaskScheduling->stopRing();
        delete mTaskScheduling;
        mTaskScheduling = nullptr;
    }
}

void AutoRunTask4426Service::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    this->silenceSend(protocal);
}
void AutoRunTask4426Service::recvStationNetCenter(const ProtocolPack& bak)
{
    /*
     * 这个通道号中心命令用不上
     */
    auto pack = bak;
    qint32 channelNo{ 1 };
    CommunicationHelper::fromByteArray(channelNo, pack.data);
    mStationNetCenter->recvStationNetCenter(pack.data);
}

void AutoRunTask4426Service::recvDeviceWorkTaskList(const ProtocolPack& pack)
{
    QList<DeviceWorkTask> deviceWorkTaskList;
    auto data = pack.data;
    data >> deviceWorkTaskList;
    if (deviceWorkTaskList.size() > 0)
    {
        mTaskScheduling->addParseDeviceTask(deviceWorkTaskList);
    }
}
void AutoRunTask4426Service::taskResourceRelease(const ManualMessage& manualMessage)
{
    ProtocolPack sendPack;
    sendPack.desID = "ServerAutoTaskService";
    sendPack.operation = "manualOperation";
    sendPack.operationACK = QString();
    sendPack.module = true;
    sendPack.data << manualMessage;
    silenceSend(sendPack);
}

void AutoRunTask4426Service::slotReadAutoRunTaskLog(const AutoTaskLogData& data)
{
    // 任务自动化运行日志入库
    TaskPlanSql::insertAutoTaskLog(data);
}
void AutoRunTask4426Service::masterSlave(const HeartbeatMessage& msg)
{
    if(mMasterStatus == msg.status)
    {
        return;
    }
    mMasterStatus = msg.status;
    /* 20211203
     * 主备机要求主备机切换任务不中断
     *
     * 这里对设备的网络已经拦截了发不出去
     * 然后再禁用日志就行
     *
     * 恢复主机的时候恢复日志就行
    */
    if(msg.status == MasterSlaveStatus::Master)
    {
        SystemLogPublish::infoMsg(QString("主备切换:当前任务调度主机%1切换中").arg(msg.currentMasterIP));
        mTaskScheduling->stopRing();
        mTaskScheduling->restoreTaskRuning();
        SystemLogPublish::infoMsg(QString("主备切换:当前任务调度主机%1切换完成").arg(msg.currentMasterIP));
    }
    else
    {
        SystemLogPublish::infoMsg("主备切换:设备备机任务调度停止中");
        mTaskScheduling->stopRing();
        SystemLogPublish::infoMsg("主备切换:设备备机任务调度停止完成");
    }
}

