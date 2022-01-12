#include "TaskPlanManagerService.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "ParseDeviceWorkTaskThread.h"
#include "RedisHelper.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskPlanSql.h"

static constexpr auto serverSelfModule = "这是发给任务运行模块的 wp??";
class TaskPlanManagerServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    static inline ProtocolPack packServerSelf(const char* operation, const char* operationACK = "", const QByteArray& data = QByteArray())
    {
        ProtocolPack sendPack;
        sendPack.desID = serverSelfModule;
        sendPack.operation = operation;
        sendPack.operationACK = operationACK;
        sendPack.module = true;
        sendPack.data = data;
        return sendPack;
    }

public:
    ParseDeviceWorkTaskThread* parseDeviceWorkTaskThread = nullptr;  // 解析设备工作计划文件
};

TaskPlanManagerService::TaskPlanManagerService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new TaskPlanManagerServiceImpl)
{
    mImpl->context = context;
    auto dbInitRes = TaskPlanSql::initDB();
    if (!dbInitRes)
    {
        qWarning() << QString("任务计划数据库初始化失败:%1").arg(dbInitRes.msg());
    }

    dbInitRes = TaskPlanSql::initAutoTaskLogDB();
    if (!dbInitRes)
    {
        qWarning() << QString("自动运行日志数据库初始化失败:%1").arg(dbInitRes.msg());
    }

    registerSubscribe("addItem", &TaskPlanManagerService::addItem, this);
    registerSubscribe("deletePlanItem", &TaskPlanManagerService::deletePlanItem, this);
    registerSubscribe("viewItem", &TaskPlanManagerService::viewItem, this);
    registerSubscribe("queryDeviceItem", &TaskPlanManagerService::queryDeviceItem, this);
    registerSubscribe("queryDTItem", &TaskPlanManagerService::queryDTItem, this);
    registerSubscribe("modifyPlanStatus", &TaskPlanManagerService::modifyPlanStatus, this);
    registerSubscribe("GetHistoryPlan", &TaskPlanManagerService::getHistoryPlan, this);
    registerSubscribe("queryAutoTaskLog", &TaskPlanManagerService::queryAutoTaskLog, this);
    registerSubscribe("editInfoItem", &TaskPlanManagerService::editInfoItem, this);
    registerSubscribe("updateItem", &TaskPlanManagerService::updateItem, this);

    initParseTaskPlanXml();
}

TaskPlanManagerService::~TaskPlanManagerService()
{
    if (mImpl->parseDeviceWorkTaskThread != nullptr)
    {
        mImpl->parseDeviceWorkTaskThread->stop();
    }

    delete mImpl;
}

void TaskPlanManagerService::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskPlanData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = TaskPlanSql::insertPlan(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加任务计划成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加任务计划失败：%1").arg(ans.msg()), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    silenceSendACK(bak);
}

void TaskPlanManagerService::deletePlanItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskDelete item;
    bak.data >> item;
    bak.data.clear();

    /* 这里会触发一个计划停止 */
    //    auto stopPlanPack = TaskPlanManagerServiceImpl::packServerSelf("StopPlanRun", "", pack.data);
    //    silenceSend(stopPlanPack);

    auto ans = TaskPlanSql::deletePlanMark(item.uuid);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除任务计划成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除任务计划失败：%1").arg(ans.msg()), bak.userID, MACRSTR(MODULE_NAME));
    }

    bak.data << ans;

    // mImpl->silenceReloadDeviceTaskWorkTask();
    silenceSendACK(bak);
}

void TaskPlanManagerService::viewItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskView item;
    bak.data >> item;
    bak.data.clear();

    if (!item.id.isEmpty())
    {
        bak.data << TaskPlanSql::queryPlanByIDMark(item.id);
    }
    else
    {
        bak.data << TaskPlanSql::queryPlanBySNMark(item.sn);
    }

    silenceSendACK(bak);
}

void TaskPlanManagerService::editInfoItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskView item;
    bak.data >> item;
    bak.data.clear();

    if (!item.id.isEmpty())
    {
        bak.data << TaskPlanSql::queryPlanByIDMark(item.id);
    }
    else
    {
        bak.data << TaskPlanSql::queryPlanBySNMark(item.sn);
    }

    silenceSendACK(bak);
}
void TaskPlanManagerService::updateItem(const ProtocolPack& pack)
{
    auto bak = pack;
    TaskPlanData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = TaskPlanSql::updatePlan(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("编辑任务计划成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("编辑任务计划失败：%1").arg(ans.msg()), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    silenceSendACK(bak);
}

void TaskPlanManagerService::queryDeviceItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskQuery item;
    bak.data >> item;
    bak.data.clear();

    bak.data << TaskPlanSql::queryHistoryDevPlanMark(item.beginDate, item.endDate);
    silenceSendACK(bak);
}
void TaskPlanManagerService::queryDTItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskQuery item;
    bak.data >> item;
    bak.data.clear();

    bak.data << TaskPlanSql::queryHistoryDTPlanMark(item.beginDate, item.endDate);
    silenceSendACK(bak);
}

void TaskPlanManagerService::modifyPlanStatus(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskUpdate item;
    bak.data >> item;
    bak.data.clear();

    bak.data << TaskPlanSql::updataTaskStatus(item.uuid, item.planStatus);

    // mImpl->silenceReloadDeviceTaskWorkTask();
    silenceSendACK(bak);
}

void TaskPlanManagerService::getHistoryPlan(const ProtocolPack& pack)
{
    auto bak = pack;
    DeviceWorkTaskConditionQuery item;
    bak.data >> item;
    bak.data.clear();

    bak.data << TaskPlanSql::getHistoryDevPlan(item);

    silenceSendACK(bak);
}

void TaskPlanManagerService::queryAutoTaskLog(const ProtocolPack& pack)
{
    auto bak = pack;

    AutoTaskLogDataCondition condition;
    bak.data >> condition;

    bak.data.clear();
    bak.data << TaskPlanSql::queryAutoTaskLog(condition.uuid, condition.startTime, condition.endTime);

    silenceSendACK(bak);
}

void TaskPlanManagerService::initParseTaskPlanXml()
{
    mImpl->parseDeviceWorkTaskThread = new ParseDeviceWorkTaskThread();
//    connect(
//        mImpl->parseDeviceWorkTaskThread, &ParseDeviceWorkTaskThread::signalNewDeviceWorkTask, this,
//        [=](const DeviceWorkTaskList& deviceWorkTaskList) {
//            //收到设备工作计划不能直接入库
//            ProtocolPack protocal;
//            protocal.srcID = "";
//            protocal.desID = "ServerAutoRunTask4426Service";
//            protocal.module = true;
//            protocal.operation = "recvDeviceWorkTaskList";
//            QByteArray data;
//            data << deviceWorkTaskList;
//            protocal.data = data;
//            this->silenceSend(protocal);
//        },
//        Qt::QueuedConnection);

//    connect(mImpl->parseDeviceWorkTaskThread, &ParseDeviceWorkTaskThread::signalNewDataTransWorkTask,
//        this, &TaskPlanManagerService::newDataTransWorkTask);

    mImpl->parseDeviceWorkTaskThread->start();
}

