#include "SubscriberHelper.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "AutoRunTaskMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "MessagePublish.h"
#include "PlatformInterface.h"
#include "StatusPersistenceMessageSerialize.h"
#include "SystemLogMessageSerialize.h"
#include "TaskPlanMessageSerialize.h"
#include "GlobalData.h"
#include "HeartbeatMessageSerialize.h"
#include "RedisHelper.h"

SubscriberHelper* SubscriberHelper::getInstance()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->addObj<SubscriberHelper>("SubscriberHelper");
}
SubscriberHelper::SubscriberHelper()
{
    m_timerID = startTimer(1000);
}
SubscriberHelper::~SubscriberHelper()
{
    if(m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
}

void SubscriberHelper::timerEvent(QTimerEvent* /*event*/)
{
    if (m_isMaster && !mSuber.is_connected())
    {
        if (RedisHelper::getInstance().getSubscriber(mSuber))
        {
            // 控制命令响应
            mSuber.subscribe(ControlCmdResponseChannel, [=](const std::string& channel, const std::string& msg) {
                Q_UNUSED(channel)

                ControlCmdResponse cmdResponse;
                QString tempMsg = QString::fromStdString(msg);
                tempMsg >> cmdResponse;

                emit signalControlCmdResponseReadable(cmdResponse);
            });

            // 系统日志
            mSuber.subscribe(SystemLogChannel, [=](const std::string& channel, const std::string& msg) {
                Q_UNUSED(channel)

                SystemLogData syslog;
                QString tempMsg = QString::fromStdString(msg);
                tempMsg >> syslog;

                emit signalSystemLogReadable(syslog);
            });

            // 任务自动化运行流程状态
            mSuber.subscribe(AutoRunTaskStepStatusChannel, [=](const std::string& channel, const std::string& msg) {
                Q_UNUSED(channel)

                CurrentRunningTaskPlanData currentRunningTaskPlanData;

                QString tempMsg = QString::fromStdString(msg);
                tempMsg >> currentRunningTaskPlanData;

                emit signalAutoRunTaskStepStatusReadable(currentRunningTaskPlanData);
            });

            // 任务自动化运行日志
            mSuber.subscribe(AutoRunTaskLogChannel, [=](const std::string& channel, const std::string& msg) {
                Q_UNUSED(channel)

                AutoTaskLogData autoTasklog;

                QString tempMsg = QString::fromStdString(msg);
                tempMsg >> autoTasklog;

                emit signalAutoRunTaskLogReadable(autoTasklog);
            });

            mSuber.commit();
        }
    }

    /* 主备 */
    auto info = GlobalData::masterSlaveInfo();
    /* 默认全是备机 */
    /* 没获取到状态 */
    if(info.status == MasterSlaveStatus::Unknown)
    {
        info.status = MasterSlaveStatus::Slave;
    }
    /* 不使用 */
    else if(!info.isUsed || info.currentMasterIP.isEmpty())
    {
        info.status = MasterSlaveStatus::Slave;
    }
    emit sg_masterSlave(info);

    m_isMaster = (info.status == MasterSlaveStatus::Master);
    if(m_masterIP == info.currentMasterIP)
    {
        return;
    }
    if(mSuber.is_connected())
    {
        mSuber.disconnect();
    }
    m_masterIP = info.currentMasterIP;
    emit sg_statusChange(info);
}
