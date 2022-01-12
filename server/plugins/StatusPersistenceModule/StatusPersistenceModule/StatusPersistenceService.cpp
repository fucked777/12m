#include "StatusPersistenceService.h"

#include "AutoTestDataTcpServer.h"
#include "CppMicroServicesUtility.h"
#include "MessagePublish.h"
#include "StatusPersistenceMessageSerialize.h"
#include "StatusSqlQuery.h"
#include "StatusTcpServer.h"
#include "SubscriberHelper.h"
#include "GlobalData.h"

class StatusPersistenceServiceImpl
{
public:
    cppmicroservices::BundleContext context;
};
StatusPersistenceService::StatusPersistenceService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new StatusPersistenceServiceImpl)
{
    m_impl->context = context;

    // 设备状态
    if (!mStatusTcpServer)
    {
        mStatusTcpServer = new StatusTcpServer;
        emit mStatusTcpServer->asynStart();
    }
    if (!m_statusSqlQuery)
    {
        m_statusSqlQuery = new StatusSqlQuery();
        m_statusSqlQuery->start();
    }

    if (mAutoTestDataTcpServer == nullptr)
    {
        mAutoTestDataTcpServer = new AutoTestDataTcpServer;
        emit mAutoTestDataTcpServer->asynStart();
    }

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_masterSlave,this,&StatusPersistenceService::masterSlave);
    //registerSubscribe("search", &StatusPersistenceService::search, this);
}

StatusPersistenceService::~StatusPersistenceService()
{
    if (logSuber.is_connected())
    {
        logSuber.disconnect();
    }

    if (mStatusTcpServer != nullptr)
    {
        mStatusTcpServer->asynStop();
    }
    if (mAutoTestDataTcpServer != nullptr)
    {
        mAutoTestDataTcpServer->asynStop();
    }
    if(m_statusSqlQuery != nullptr)
    {
        m_statusSqlQuery->stop();
        delete m_statusSqlQuery;
    }
    delete m_impl;
}

//void StatusPersistenceService::search(const ProtocolPack& pack)
//{
//    auto tempPack = pack;
//    DeviceStatusDataCondition statusDataCondition;
//    tempPack.data >> statusDataCondition;

//    if (m_statusSqlQuery)
//    {
//        DeviceStatusLogDataList dataList;

//        tempPack.data.clear();
//        dataList = m_statusSqlQuery
//                       ->search(statusDataCondition.devices, statusDataCondition.units, statusDataCondition.sids, statusDataCondition.start_time,
//                                statusDataCondition.end_time)
//                       .value();
//        tempPack.data << dataList;
//        silenceSendACK(tempPack);
//    }
//}

void StatusPersistenceService::masterSlave(const HeartbeatMessage& msg)
{
    if(msg.status != MasterSlaveStatus::Master)
    {
        if (logSuber.is_connected())
        {
            logSuber.disconnect();
        }
        return;
    }
    if(logSuber.is_connected())
    {
        return;
    }

    if (msg.status == MasterSlaveStatus::Master && RedisHelper::getInstance().getSubscriber(logSuber))
    {
        // 设备状态
        logSuber.subscribe(DeviceStatusDataChannel, [=](const std::string& /*channel*/, const std::string& msg) {
            DeviceStatusData statusData;
            QString currentData = QString::fromStdString(msg);
            currentData >> statusData;
            int deviceId;
            statusData.deviceID >> deviceId;

            emit mStatusTcpServer->statusResultReceived(deviceId, currentData);
            m_statusSqlQuery->saveStatusResult(deviceId, currentData);
        });

        // 自动化测试数据
        logSuber.subscribe(AutoTestDataChannel, [=](const std::string& /*channel*/, const std::string& msg) {
            auto data = QString::fromStdString(msg);
            if (mAutoTestDataTcpServer != nullptr)
            {
                emit mAutoTestDataTcpServer->autoTestDataReceived(data);
            }
        });

        logSuber.commit();
    }
}
