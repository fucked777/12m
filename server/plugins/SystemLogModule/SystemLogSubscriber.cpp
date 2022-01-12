#include "SystemLogSubscriber.h"

#include "MessagePublish.h"
#include "SqlSystemLogManager.h"
#include "SubscriberHelper.h"
#include "SystemLogMessageSerialize.h"

#include <QDebug>

SystemLogSubscriber::SystemLogSubscriber()
{
    auto initDBRes = sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("系统日志初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalSystemLogReadable, this, &SystemLogSubscriber::slotReadSystemLog);
}

void SystemLogSubscriber::slotReadSystemLog(const SystemLogData& data)
{
    SystemLogData temp = data;
    if (temp.module.trimmed().isEmpty())
    {
        temp.module = "AutoRecordLogModule";
    }
    sql.insertSysLog(temp);
}
