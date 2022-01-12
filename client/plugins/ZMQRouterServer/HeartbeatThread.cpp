#include "HeartbeatThread.h"

#include "BundleContext.h"
#include "GlobalData.h"
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

class HeartbeatThreadImpl
{
public:
    cppmicroservices::BundleContext context;
};


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

HeartbeatThread::HeartbeatThread(cppmicroservices::BundleContext context)
    : QThread(nullptr)
    , m_impl(new HeartbeatThreadImpl)
{
    m_impl->context = context;
}

HeartbeatThread::~HeartbeatThread() { stopHeartbeat(); }

void HeartbeatThread::run()
{
    while (!isInterruptionRequested())
    {
        /* 主备 */
        QString value;
        RedisHelper::getInstance().getData("HA_STATUS", value);

        HeartbeatMessage m_haMsg;
        value >> m_haMsg;
        if (!m_haMsg.currentMasterIP.isEmpty())
        {
            GlobalData::setMasterSlaveInfo(m_haMsg);
            //RedisHelper::getInstance().resetConnectHost(m_haMsg.currentMasterIP);
        }
        QThread::sleep(2);
    }
}

void HeartbeatThread::startHeartbeat()
{
    SubscriberHelper::getInstance();
    start();
}

void HeartbeatThread::stopHeartbeat()
{
    requestInterruption();
    quit();
    wait();
}
