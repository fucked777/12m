#include "TaskExecuteLogService.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "RedisHelper.h"
#include "TaskPlanMessageSerialize.h"

class TaskExecuteLogServiceImpl
{
public:
    cppmicroservices::BundleContext context;
    //    SqlTaskExecuteLogManager sql;
    //    int timerID{ -1 };

    //    static void pushDeviceData(const DeviceWorkTaskList& data);
    //    Optional<DeviceWorkTaskList> reloadDevice(bool push = true);
    //    void silenceReloadDevice(bool push = true);

    //    static void pushDataTransSSData(const QList<DataTransWorkTask>& data);
    //    Optional<QList<DataTransWorkTask>> reloadDataSSTrans(bool push = true);
    //    void silenceReloadDataSSTrans(bool push = true);

    //    static void pushDataTransSHData(const QList<DataTransWorkTask>& data);
    //    Optional<QList<DataTransWorkTask>> reloadDataSHTrans(bool push = true);
    //    void silenceReloadDataSHTrans(bool push = true);
};

TaskExecuteLogService::TaskExecuteLogService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new TaskExecuteLogServiceImpl)
{
    mImpl->context = context;

    registerSubscribe("GetCurrentPageDeviceInfo", &TaskExecuteLogService::GetCurrentPageDeviceInfo, this);

    // mImpl->timerID = startTimer(6000);
}

TaskExecuteLogService::~TaskExecuteLogService() { delete mImpl; }

void TaskExecuteLogService::timerEvent(QTimerEvent* event)
{
    //    mImpl->silenceReloadDevice();
    //    mImpl->silenceReloadDataSHTrans();
    //    mImpl->silenceReloadDataSSTrans();
}

void TaskExecuteLogService::GetAllDeviceInfo(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetAllDeviceInfo();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetCurrentPageDeviceInfo(const ProtocolPack& pack)
{
    //    auto bak = pack;

    //    DeviceWorkTaskCondition condition;
    //    bak.data >> condition;

    //    bak.data.clear();
    //    bak.data << mImpl->sql.GetCurrentPageDeviceInfo(condition.totalCount, condition.pageSize, condition.currentPage).value();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::InsertDeviceWorkTask(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    DeviceWorkTask item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.InsertDeviceWorkTask(item);
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDevice();
}

void TaskExecuteLogService::UpdataTaskStatus(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    DeviceWorkTask item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.UpdataTaskStatus();
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDevice();
}

void TaskExecuteLogService::DeleteDeviceWorkTask(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    DeviceWorkTask item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.DeleteDeviceWorkTask(item);
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDevice();
}

void TaskExecuteLogService::GetTodayDeviceWorkTaskInfo(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetTodayDeviceWorkTaskInfo();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetHistoryPlan(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    //    bak.data << mImpl->sql.GetHistoryPlan();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetRunningOrNoStartDeviceInfo(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetRunningOrNoStartDeviceInfo();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetAllDataTranPlanInfoSS(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetAllDataTranPlanInfoSS();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetAllDataTranPlanInfoSH(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetAllDataTranPlanInfoSH();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetCurrentPageDataTranPlanInfoSS(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetAllDataTranPlanInfoSH();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::GetCurrentPageDataTranPlanInfoSH(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.GetAllDataTranPlanInfoSH();

    //    silenceSendACK(bak);
}

void TaskExecuteLogService::InsertDataTranPlan(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    DataTransWorkTask item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.InsertDataTranPlan(item);
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDataSSTrans();
    //    mImpl->silenceReloadDataSHTrans();
}

void TaskExecuteLogService::InsertPlanTask(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    QList<DataTransWorkTask> item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.InsertPlanTask(item);
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDataSSTrans();
    //    mImpl->silenceReloadDataSHTrans();
}

void TaskExecuteLogService::DeleteDataTranPlan(const ProtocolPack& pack)
{
    //    auto bak = pack;
    //    DataTransWorkTask item;
    //    bak.data >> item;
    //    bak.data.clear();

    //    bak.data << mImpl->sql.DeleteDataTranPlan(item);
    //    silenceSendACK(bak);

    //    mImpl->silenceReloadDataSSTrans();
    //    mImpl->silenceReloadDataSHTrans();
}

// void TaskExecuteLogServiceImpl::pushDeviceData(const DeviceWorkTaskList& data)
//{
//    JsonWriter writer;
//    writer& data;
//    RedisHelper::getInstance().setData("DeviceWorkTaskInfo", writer.GetString());
//}

// Optional<DeviceWorkTaskList> TaskExecuteLogServiceImpl::reloadDevice(bool push)
//{
//    auto itemMap = sql.GetAllDeviceInfo();
//    if (itemMap.success() & push)
//    {
//        pushDeviceData(itemMap.value());
//    }

//    return itemMap;
//}

// void TaskExecuteLogServiceImpl::silenceReloadDevice(bool push)
//{
//    auto itemMap = reloadDevice(push);
//    if (!itemMap)
//    {
//        auto msg = QString("查询设备信息错误:%1").arg(itemMap.msg());
//        // Error(itemMap.errorCode(), msg);
//        return;
//    }
//}

// void TaskExecuteLogServiceImpl::pushDataTransSSData(const QList<DataTransWorkTask>& data)
//{
//    JsonWriter writer;
//    writer& data;
//    RedisHelper::getInstance().setData("DataTranPlanInfoSS", writer.GetString());
//}

// Optional<QList<DataTransWorkTask>> TaskExecuteLogServiceImpl::reloadDataSSTrans(bool push)
//{
//    auto itemMap = sql.GetAllDataTranPlanInfoSS();
//    if (itemMap.success() & push)
//    {
//        pushDataTransSSData(itemMap.value());
//    }

//    return itemMap;
//}

// void TaskExecuteLogServiceImpl::silenceReloadDataSSTrans(bool push)
//{
//    auto itemMap = reloadDataSSTrans(push);
//    if (!itemMap)
//    {
//        auto msg = QString("查询实时数传信息错误:%1").arg(itemMap.msg());
//        // Error(itemMap.errorCode(), msg);
//        return;
//    }
//}

// void TaskExecuteLogServiceImpl::pushDataTransSHData(const QList<DataTransWorkTask>& data)
//{
//    JsonWriter writer;
//    writer& data;
//    RedisHelper::getInstance().setData("DataTranPlanInfoSH", writer.GetString());
//}

// Optional<QList<DataTransWorkTask>> TaskExecuteLogServiceImpl::reloadDataSHTrans(bool push)
//{
//    auto itemMap = sql.GetAllDataTranPlanInfoSH();
//    if (itemMap.success() & push)
//    {
//        pushDataTransSHData(itemMap.value());
//    }

//    return itemMap;
//}

// void TaskExecuteLogServiceImpl::silenceReloadDataSHTrans(bool push)
//{
//    auto itemMap = reloadDataSHTrans(push);
//    if (!itemMap)
//    {
//        auto msg = QString("查询事后数传信息错误:%1").arg(itemMap.msg());
//        // Error(itemMap.errorCode(), msg);
//        return;
//    }
//}
