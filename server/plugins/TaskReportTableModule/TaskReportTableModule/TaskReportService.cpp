#include "TaskReportService.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "SqlTaskReportManager.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskReportMessageSerialize.h"

using DeviceWorkTaskList = QList<DeviceWorkTask>;
class TaskReportServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlTaskReportManager sql;
    //在这个帮助类里面不提供redis的数据刷新操作
};

TaskReportService::TaskReportService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new TaskReportServiceImpl)
{
    mImpl->context = context;

    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("任务报表管理初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }
    registerSubscribe("insertData", &TaskReportService::insertData, this);
    registerSubscribe("getDataByUUID", &TaskReportService::getDataByUUID, this);
}

TaskReportService::~TaskReportService() { delete mImpl; }

void TaskReportService::insertData(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskReportTable item;
    bak.data >> item;
    bak.data.clear();

    bak.data << mImpl->sql.insertData(item);
    silenceSendACK(bak);
}

void TaskReportService::getDataByUUID(const ProtocolPack& pack)
{
    auto bak = pack;
    QString uuid;
    JsonReader reader(bak.data);
    reader& uuid;

    bak.data.clear();
    bak.data << mImpl->sql.getDataByUUID(uuid).value();
    silenceSendACK(bak);
}
