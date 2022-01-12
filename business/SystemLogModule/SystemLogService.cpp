#include "SystemLogService.h"
#include "CppMicroServicesUtility.h"

#include "SqlSystemLogManager.h"
#include "SubscriberHelper.h"
#include "SystemLogMessageSerialize.h"

#include <QDebug>

class SystemLogServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    SqlSystemLogManager sql;
};

SystemLogService::SystemLogService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new SystemLogServiceImpl)
{
    mImpl->context = context;

    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("系统日志初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }
}

SystemLogService::~SystemLogService() { delete mImpl; }

void SystemLogService::slotLogConditionQuery(const QByteArray& data)
{
    QByteArray tempData = data;
    SystemLogCondition range;
    tempData >> range;
    quint64 startTime = range.startTime.toULongLong();
    quint64 endTime = range.endTime.toULongLong();
    int level = static_cast<int>(range.level);
    int currentPage = range.currentPage;
    int pageSize = range.pageSize;

    SystemLogACK ack;
    int totalCount = mImpl->sql.totalNumberOfQueries(startTime, endTime, level).value();
    if (totalCount)
    {
        ack = mImpl->sql.logConditionQuery(startTime, endTime, level, currentPage, pageSize).value();
        ack.totalCount = totalCount;
    }
    QByteArray resultData;
    resultData << ack;
    emit signalLogConditionQueryACK(resultData);
}
