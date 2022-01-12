#include "HeartbeatThread.h"

#include "BundleContext.h"
#include "GlobalData.h"
#include "HeartBeatXMLReader.h"
#include "HeartbeatMessageSerialize.h"
#include "PlatformInterface.h"
#include "RedisHelper.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QSettings>
#include "SubscriberHelper.h"

class HeartbeatThreadImpl : public QThread
{
public:
    HeartbeatThreadImpl(QObject* parent = nullptr);
    ~HeartbeatThreadImpl();

private:
    void run() override;
};

HeartbeatThreadImpl::HeartbeatThreadImpl(QObject* parent)
    : QThread(parent)
{
}

HeartbeatThreadImpl::~HeartbeatThreadImpl() {}
void HeartbeatThreadImpl::run()
{
#ifdef TEST_MASTER_SLAVE
    return;
#endif

    HeartBeatInfo heartbeatInfo;
    HeartBeatXMLReader heartbeatXmlReader;
    heartbeatXmlReader.getHeartbeatConfig(heartbeatInfo);

    HeartbeatMessage haMsg;
    haMsg.serverID = PlatformConfigTools::instanceID();
    haMsg.isUsed = heartbeatInfo.isUsed;

    auto haFunc = [&]()
    {
        bool isMaster = true;
        if (heartbeatInfo.isUsed)

        {
            // 这里实现心跳检测
            auto list = QNetworkInterface::allAddresses();
            QSet<QString> ipSet;
            for (auto& item : list)
            {
                ipSet << item.toString();
            }
            isMaster = ipSet.contains(heartbeatInfo.vIP);
            if (isMaster && ipSet.contains(heartbeatInfo.aIP))
            {
                haMsg.currentMasterIP = heartbeatInfo.aIP;
            }
            else if (isMaster && ipSet.contains(heartbeatInfo.bIP))
            {
                haMsg.currentMasterIP = heartbeatInfo.bIP;
            }
            else
            {
                isMaster = false;
            }
        }
        return isMaster;
    };

    /* 先执行一次 */
    if(haFunc())
    {
        GlobalData::setMasterSlaveInfo(haMsg);
    }
    while (!isInterruptionRequested())
    {
        auto isMaster = haFunc();
        haMsg.status = isMaster ? MasterSlaveStatus::Master : MasterSlaveStatus::Slave;

        if (isMaster)
        {
            /* 当前程序内部接口获取主备机 */
            GlobalData::setMasterSlaveInfo(haMsg);

            /* Redis推送 */
            QString jsonData;
            jsonData << haMsg;
            RedisHelper::getInstance().setData("HA_STATUS", jsonData);
        }
        else
        {
            QString jsonData;
            RedisHelper::getInstance().getData("HA_STATUS", jsonData);
            jsonData >> haMsg;

            haMsg.status = MasterSlaveStatus::Slave;
            GlobalData::setMasterSlaveInfo(haMsg);
        }

        QThread::sleep(1);
    }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

HeartbeatThread::HeartbeatThread(cppmicroservices::BundleContext /*context*/, QObject* parent)
    : INetMsg(parent)
    , m_impl(new HeartbeatThreadImpl)
{
}

HeartbeatThread::~HeartbeatThread() { stopHeartbeat(); }

void HeartbeatThread::startHeartbeat()
{
    SubscriberHelper::getInstance();
    m_impl->start();
}

void HeartbeatThread::stopHeartbeat()
{
    m_impl->requestInterruption();
    m_impl->quit();
    m_impl->wait();
}
