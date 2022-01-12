#include "TimeService.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "TimeInfoXMLReader.h"
#include "TimeMessageSerialize.h"
#include "TimeSyncThread.h"
#include "SubscriberHelper.h"
#include "HeartbeatMessageSerialize.h"
#include "MessagePublish.h"

static constexpr auto serverSelfModule = "ClientTimeService";

class TimeServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    TimeSyncThread* timeSyncThread = nullptr;
    MasterSlaveStatus masterStatus{MasterSlaveStatus::Unknown};

    static inline ProtocolPack packServerSelf(const char* operation, const char* operationACK = "", const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = serverSelfModule;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = false;
        sendPack.data = data;
        return sendPack;
    }
};

TimeService::TimeService(cppmicroservices::BundleContext context)
    : INetMsg("TimeServiceThread", nullptr)
    , mImpl(new TimeServiceImpl)
{
    mImpl->context = context;

    registerSubscribe("SlotSaveTimeConfigInfo", &TimeService::slotSaveTimeConfigInfo, this);

    TimeInfoXMLReader reader;
    TimeConfigInfo timeConfigInfo;
    reader.getTimeConfigInfo(timeConfigInfo);
    GlobalData::setTimeConfigInfo(timeConfigInfo);                  // 本地配置文件
    GlobalData::setUsedTimeTypeRedis(timeConfigInfo.usedTimeType);  // redis时间类型
    GlobalData::setUsedTimeType(timeConfigInfo.usedTimeType);       // 本地时间类型

    mImpl->timeSyncThread = new TimeSyncThread;
    // 发送时间信息到客户端
    //connect(mImpl->timeSyncThread, &TimeSyncThread::signalTimeUpdated, this, &TimeService::slotTimeUpdated);
    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_masterSlave,this,&TimeService::masterSlave);
}

TimeService::~TimeService()
{
    if (mImpl->timeSyncThread != nullptr)
    {
        mImpl->timeSyncThread->stopSync();
        delete mImpl->timeSyncThread;
        mImpl->timeSyncThread = nullptr;
    }

    delete mImpl;
}

void TimeService::slotSaveTimeConfigInfo(const ProtocolPack& pack)
{
    auto json = pack.data;

    TimeConfigInfo info;
    json >> info;

    TimeInfoXMLReader reader;
    reader.saveTimeConfigInfo(info);
}

void TimeService::slotTimeUpdated(const TimeData& timeData)
{
    // 服务器保存时间信息
    GlobalData::setTimeData(timeData);

    QByteArray json;
    json << timeData;

    // 发送给客户端
    auto pack = TimeServiceImpl::packServerSelf("timeDataUpdated", "", json);
    silenceSend(pack);
}
void TimeService::masterSlave(const HeartbeatMessage& msg)
{
    if(mImpl->masterStatus == msg.status)
    {
        return;
    }
    mImpl->masterStatus = msg.status;
    /* 主机先停止再启动
     * 备机直接停止
    */

    if(msg.status == MasterSlaveStatus::Master)
    {
        SystemLogPublish::infoMsg(QString("主备切换:%1主机时间同步重置中").arg(msg.currentMasterIP));
        mImpl->timeSyncThread->stopSync();
        mImpl->timeSyncThread->startSync();
        SystemLogPublish::infoMsg(QString("主备切换:%1主机时间同步重置完成").arg(msg.currentMasterIP));
    }
    else
    {
        SystemLogPublish::infoMsg(QString("主备切换:备机时间同步停止中"));
        mImpl->timeSyncThread->stopSync();
        SystemLogPublish::infoMsg(QString("主备切换:备机时间同步停止完成"));
    }
}

