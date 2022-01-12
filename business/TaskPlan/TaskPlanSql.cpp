#include "TaskPlanSql.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "DBInterface.h"
#include "SqlGlobalData.h"
#include "TaskPlanMessageSerialize.h"
#include "Utility.h"
#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTextCodec>
#include <QUuid>

/* 设备工作计划的uuid是流水号_任务号 */
static int snTruncation = 8;
static char snDateFormat[] = "yyyyMMdd";           // 流水号格式化字符串
static char snDateFormatFirst[] = "yyyyMMdd0001";  // 流水号格式化字符串
static int dayRetrogression = -20;                 /* 这里倒退20天因为计划一般不会提前太久下 */
static char sqlGlobalDataPlanIDName[] = "PlanID";  // 记录当前最大的任务流水号的表的数据

class TaskPlanSqlPrivate
{
public:
    /* 既是唯一ID也是计划流水号 */
    static Optional<QString> createPlanID(QSqlDatabase& db);
    static bool snExist(qulonglong item, QSqlDatabase& db);
    /* 数据库字段转结构体 */
    static DataTranWorkTask sqlQueryToDTData(QSqlQuery& sqlQuery);
    static DeviceWorkTask sqlQueryToDevData(QSqlQuery& sqlQuery);
    /* 初始化数据库 */
    static OptionalNotValue initDevicePlanDB(QSqlDatabase& db);
    static OptionalNotValue initDTPlanDB(QSqlDatabase& db);
    /* 创建数传的sql语句占位符,方便一次插入多条数据 */
    static QString getParamPlaceholder(int count);

    /* 插入数据 所有的数据插入世时 删除标记被标记为 0 */
    static Optional<TaskPlanData> insertPlan(QSqlDatabase& db, const TaskPlanData& bak, bool isCreatUuid);
    static Optional<DataTranWorkTask> insertPlan(QSqlDatabase& db, const DataTranWorkTask& pDeviceWorkTask);
    static Optional<DeviceWorkTask> insertPlan(QSqlDatabase& db, const DeviceWorkTask& pDeviceWorkTask);
    static Optional<DataTranWorkTaskList> insertPlan(QSqlDatabase& db, const DataTranWorkTaskList& pDeviceWorkTask);

    /*
     * 检查任务是否可以更新数据
     * Optional<bool> 代表查询是否成功
     * 成功后bool代表是否在运行
     */
    static Optional<bool> checkTaskIsUpdate(QSqlDatabase& db, const QString& uuid);

    /* 删除计划 从数据库彻底删除数据 */
    static OptionalNotValue deletePlan(QSqlDatabase& db, const QString& deviceTaskID);
    /* 删除计划 不会将数据直接删除 只会将当前数据标记为已删除 */
    static OptionalNotValue deletePlanMark(QSqlDatabase& db, const QString& deviceTaskID);

    /* 根据ID查询设备工作计划 */
    static Optional<DeviceWorkTask> queryPlanByID(QSqlDatabase& db, const QString& deviceTaskID);
    static Optional<DeviceWorkTask> queryPlanByIDMark(QSqlDatabase& db, const QString& deviceTaskID);

    /* 根据设备工作计划查询相关联的数传工作计划 */
    static Optional<DataTranWorkTaskList> queryPlanByDev(QSqlDatabase& db, const DeviceWorkTask& deviceWorkTask);
    static Optional<DataTranWorkTaskList> queryPlanByDevMark(QSqlDatabase& db, const DeviceWorkTask& deviceWorkTask);

    /* 根据流水号查询设备工作计划 */
    static Optional<DeviceWorkTaskList> queryPlanBySN(QSqlDatabase& db, const QString& sn);
    static Optional<DeviceWorkTaskList> queryPlanBySNMark(QSqlDatabase& db, const QString& sn);

    /* 根据流水号和跟踪号查询数传工作计划 */
    static Optional<DataTranWorkTaskList> queryPlanBySN(QSqlDatabase& db, const QString& sn, const QString& traceNum);
    static Optional<DataTranWorkTaskList> queryPlanBySNMark(QSqlDatabase& db, const QString& sn, const QString& traceNum);
};

#if 0

Optional<QString> TaskPlanSqlPrivate::createPlanID(QSqlDatabase& db)
{
    using ResType = Optional<QString>;
    auto curDate = GlobalData::currentDate(); /* 今天的日期 */
    QString sql = "select m_uuid  from DeviceWorkTask order by m_uuid desc limit 1";
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    if (!sqlQuery.next())
    {
        /* 没查出数据,那就是空的,直接来一个0001 */
        return ResType(curDate.toString(snDateFormatFirst));
    }

    bool isOK = false;
    auto record = sqlQuery.record();
    auto maxUuid = record.field("m_uuid").value().toString();
    auto snList = maxUuid.split("_"); /* 分解后应该得到两个数据 一个是计划流水号,另一个是任务编号 */
    /* 这里存储格式不正确 */
    if (snList.size() != 2)
    {
        auto errMsg = QString("数据库中有错误的数据:%1").arg(maxUuid);
        return ResType(ErrorCode::InvalidData, errMsg);
    }
    /* 这里校验一下流水号是否是当天的,不是就不使用这个
     * 流水号=日期(snDateFormat)+4位数字编号  数字编号依次递增
     * 所以这里要先提取日期
     */
    auto idDate = snList.at(0).left(snTruncation);
    auto queryIDDate = QDate::fromString(idDate, snDateFormat);

    /* 相等那就是今天的加一就行 */
    /* 比今天的还大,那完蛋了,时间出错了,使用查出来的值加一就行 */
    if (queryIDDate >= curDate)
    {
        auto maxNum = snList.at(0).toULongLong(&isOK) + 1;
        /* 转换失败,这个是,数据库出错了,因为按照预设规则是肯定能转换成数字的 */
        if (!isOK)
        {
            auto errMsg = QString("数据库中有错误的数据:%1").arg(maxUuid);
            return ResType(ErrorCode::InvalidData, errMsg);
        }
        return ResType(QString::number(maxNum));
    }

    /* 之前的日期,那就直接使用今天的 */
    return ResType(curDate.toString(snDateFormatFirst));
}
#else
Optional<QString> TaskPlanSqlPrivate::createPlanID(QSqlDatabase& db)
{
    using ResType = Optional<QString>;
    auto curDate = GlobalData::currentDate(); /* 今天的日期 */

    /* 1. 获取记录的最大流水号 */
    auto recordResult = SqlGlobalData::selectItem(db, sqlGlobalDataPlanIDName);
    if (!recordResult)
    {
        return ResType(ErrorCode::SqlExecFailed, recordResult.msg());
    }

    /* 2. 获取计划中的最大流水号 */
    QString sql = "select m_uuid  from DeviceWorkTask order by m_uuid desc limit 1";
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    auto recordPlanNumStr = QString(recordResult.value());
    /* 2.1
     * 如果计划中的最大流水号为空则使用记录的流水号做判断
     * 否则将两个流水号作比较
     */
    QString maxPlanNumString;
    if (!sqlQuery.next())
    {
        maxPlanNumString = recordPlanNumStr;
    }
    else
    {
        auto record = sqlQuery.record();
        auto maxUuid = record.field("m_uuid").value().toString();
        auto snList = maxUuid.split("_"); /* 分解后应该得到两个数据 一个是计划流水号,另一个是任务编号 */
        /* 这里存储格式不正确 */
        if (snList.size() != 2)
        {
            auto errMsg = QString("数据库中有错误的数据:%1").arg(maxUuid);
            return ResType(ErrorCode::InvalidData, errMsg);
        }
        /* 单纯字符串比较会有问题 */
        bool success1 = false;
        bool success2 = false;
        auto temp1 = recordPlanNumStr.toULongLong(&success1);
        auto temp2 = snList.at(0).toULongLong(&success2);
        if (!success1)
        {
            auto errMsg = QString("当前流水号错误:%1").arg(recordPlanNumStr);
            return ResType(ErrorCode::InvalidData, errMsg);
        }
        if (!success2)
        {
            auto errMsg = QString("当前流水号错误:%1").arg(snList.at(0));
            return ResType(ErrorCode::InvalidData, errMsg);
        }

        maxPlanNumString = (temp1 > temp2 ? recordPlanNumStr : snList.at(0));
    }

    /* 3 选出是那个流水号最大了,然后开始比较校验 */

    /* 这里校验一下流水号是否是当天的,不是就不使用这个
     * 流水号=日期(snDateFormat)+4位数字编号  数字编号依次递增
     * 所以这里要先提取日期
     */

    auto queryIDDate = QDate::fromString(maxPlanNumString.left(snTruncation), snDateFormat);
    /* 相等那就是今天的加一就行 */
    /* 比今天的还大,那完蛋了,时间出错了,使用查出来的值加一就行 */
    qulonglong maxNum = 0;
    if (queryIDDate >= curDate)
    {
        bool isOK = false;
        maxNum = maxPlanNumString.toULongLong(&isOK) + 1;
        /* 转换失败,这个是,数据库出错了,因为按照预设规则是肯定能转换成数字的 */
        if (!isOK)
        {
            auto errMsg = QString("数据库中有错误的数据:%1").arg(maxPlanNumString);
            /* 这里有一种情况是全局数据错一次然后就永远无法正确了 这里清除一下 */
            SqlGlobalData::clearItem(db, sqlGlobalDataPlanIDName);
            return ResType(ErrorCode::InvalidData, errMsg);
        }
    }
    else
    {
        maxNum = curDate.toString(snDateFormatFirst).toULongLong();
    }

    /* 判是否存在直到不存在为止 */
    while (snExist(maxNum, db))
    {
        ++maxNum;
    }

    /* 之前的日期,那就直接使用今天的 */
    return ResType(QString::number(maxNum));
}

#endif
bool TaskPlanSqlPrivate::snExist(qulonglong maxNum, QSqlDatabase& db)
{
    auto sql = QString("select * from DeviceWorkTask where m_uuid like '%1%' ").arg(maxNum);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
DataTranWorkTask TaskPlanSqlPrivate::sqlQueryToDTData(QSqlQuery& sqlQuery)
{
    auto recordData = sqlQuery.record();

    DataTranWorkTask item;
    item.m_uuid = recordData.field("m_uuid").value().toString();
    item.m_create_userid = recordData.field("m_create_userid").value().toString();
    item.m_createTime = recordData.field("m_createTime").value().toDateTime();
    item.m_task_start_time = recordData.field("m_task_start_time").value().toDateTime();
    item.m_task_end_time = recordData.field("m_task_end_time").value().toDateTime();
    item.m_transport_time = recordData.field("m_transport_time").value().toDateTime();
    item.m_task_commit_time = recordData.field("m_task_commit_time").value().toDateTime();
    item.m_plan_serial_number = recordData.field("m_plan_serial_number").value().toString();
    item.m_work_unit = recordData.field("m_work_unit").value().toString();
    item.m_equipment_no = recordData.field("m_equipment_no").value().toString();
    item.m_task_total_num = recordData.field("m_task_total_num").value().toInt();
    item.m_task_status = TaskPlanStatus(recordData.field("m_task_status").value().toInt());
    item.m_working_mode = DataTransMode(recordData.field("m_working_mode").value().toInt());
    item.m_task_code = recordData.field("m_task_code").value().toString();
    item.m_dt_transNum = recordData.field("m_dt_transNum").value().toString();
    item.m_channelNum = recordData.field("m_channelNum").value().toString();
    item.m_datatrans_center = recordData.field("m_datatrans_center").value().toString();
    item.m_center_protocol = CenterProtocol(recordData.field("m_center_protocol").value().toInt());
    item.m_datatrans_center_id = recordData.field("m_datatrans_center_id").value().toString();
    item.m_priority = DataTransPriority(recordData.field("m_priority").value().toInt());
    item.m_transport_protocols = recordData.field("m_transport_protocols").value().toString();
    item.m_ring_num = recordData.field("m_ring_num").value().toInt();
    item.m_source = TaskPlanSource(recordData.field("m_source").value().toInt());
    item.m_belongTarget = recordData.field("m_belongTarget").value().toInt();
    return item;
}
DeviceWorkTask TaskPlanSqlPrivate::sqlQueryToDevData(QSqlQuery& sqlQuery)
{
    auto recordData = sqlQuery.record();

    DeviceWorkTask item;
    item.m_uuid = recordData.field("m_uuid").value().toString();
    item.m_create_userid = recordData.field("m_create_userid").value().toString();
    item.m_createTime = recordData.field("m_createTime").value().toDateTime();
    item.m_preStartTime = recordData.field("m_preStartTime").value().toDateTime();
    item.m_startTime = recordData.field("m_startTime").value().toDateTime();
    item.m_endTime = recordData.field("m_endTime").value().toDateTime();
    item.m_plan_serial_number = recordData.field("m_plan_serial_number").value().toString();
    item.m_work_unit = recordData.field("m_work_unit").value().toString();
    item.m_equipment_no = recordData.field("m_equipment_no").value().toString();
    item.m_scheduling_mode = recordData.field("m_scheduling_mode").value().toString();
    item.m_task_total_num = recordData.field("m_task_total_num").value().toInt();
    item.m_is_zero = TaskCalibration(recordData.field("m_is_zero").value().toInt());
    item.m_calibration = TaskCalibration(recordData.field("m_calibration").value().toInt());
    item.m_working_mode = recordData.field("m_working_mode").value().toString();
    item.m_target_number = recordData.field("m_target_number").value().toInt();
    item.m_lord_with_target = recordData.field("m_lord_with_target").value().toString();
    item.m_tracking_order = recordData.field("m_tracking_order").value().toString();
    item.m_task_status = TaskPlanStatus(recordData.field("m_task_status").value().toInt());
    item.m_source = TaskPlanSource(recordData.field("m_source").value().toInt());

    auto jsonData = recordData.field("m_totalTargetMap").value().toByteArray();
    jsonData >> item.m_totalTargetMap;
    return item;
}

/*
 * 任务流水号的创建
 * 之前的策略 就是if 0 里面的
 * 直接在计划里面根据uuid去找最大值
 * 此方法有一个缺陷，在界面创建的时候
 * 比如已经创建了一个计划名叫A 他的流水号是1
 * 此时删除A再创建一个计划叫B 他的流水号也是1
 *
 * 所以需要有一个记录当前最大流水号的字段
 * 这里直接复用以前预留的表GlobalData
 * 在里面加入一行数据记录当前的最大流水号
 *
 * 首先查询任务计划的最大流水号 为 N1
 * 然后查询记录的最大流水号    为 N2
 * 比较两个流水号选最大的 执行之前的流水号比较的策略
 *
 * 最后将最终的流水号写入记录中
 *
 * 20210622 增加删除任务时不删除数据只是将数据标记为删除
 * 但是总会有一个地方会真正删除数据的
 * 不然数据会越来越多
 * 所以这里保留全局数据的记录当前最大的任务流水号
 */

OptionalNotValue TaskPlanSqlPrivate::initDevicePlanDB(QSqlDatabase& db)
{
    /* 这里uuid=m_plan_serial_number_任务编号
     * m_isDelete
     * 默认为0
     * 否则则认为是已经标记为删除了
     */
    QString sql = "create table if not exists DeviceWorkTask("
                  "m_uuid               varchar(64) primary key     ,"
                  "m_create_userid      text                        ,"
                  "m_createTime         TIMESTAMP  not null         ,"
                  "m_preStartTime       TIMESTAMP                   ,"
                  "m_startTime          TIMESTAMP  not null         ,"
                  "m_endTime            TIMESTAMP                   ,"
                  "m_plan_serial_number varchar(128) not null       ,"
                  "m_work_unit          text                        ,"
                  "m_equipment_no       text                        ,"
                  "m_scheduling_mode    text                        ,"
                  "m_task_total_num     int                         ,"
                  "m_is_zero            int                         ,"
                  "m_calibration        int                         ,"
                  "m_working_mode       text                        ,"
                  "m_target_number      int                         ,"
                  "m_lord_with_target   text                        ,"
                  "m_tracking_order     text                        ,"
                  "m_task_status        int                         ,"
                  "m_source             int                         ,"
                  "m_totalTargetMap     blob                        ,"
                  "m_isDelete           int                         );";

    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
    }

    return OptionalNotValue();
}
OptionalNotValue TaskPlanSqlPrivate::initDTPlanDB(QSqlDatabase& db)
{
    /*
     * m_isDelete
     * 默认为0
     * 否则则认为是已经标记为删除了
     */
    QString sql = "create table if not exists DataTranWorkTask("
                  "m_uuid                 varchar(64)           ,"
                  "m_create_userid        text                  ,"
                  "m_createTime           TIMESTAMP             ,"
                  "m_task_start_time      TIMESTAMP             ,"
                  "m_task_end_time        TIMESTAMP             ,"
                  "m_transport_time       TIMESTAMP             ,"
                  "m_task_commit_time     TIMESTAMP             ,"
                  "m_plan_serial_number   varchar(128) not null ,"
                  "m_work_unit            text                  ,"
                  "m_equipment_no         text                  ,"
                  "m_task_total_num       int                   ,"
                  "m_task_status          int                   ,"
                  "m_working_mode         varchar(64)           ,"
                  "m_task_code            text                  ,"
                  "m_dt_transNum          text                  ,"
                  "m_channelNum           text                  ,"
                  "m_datatrans_center     text                  ,"
                  "m_center_protocol      text                  ,"
                  "m_datatrans_center_id  text                  ,"
                  "m_priority             int                   ,"
                  "m_transport_protocols  text                  ,"
                  "m_ring_num             int                   ,"
                  "m_source               int                   ,"
                  "m_belongTarget         int                   ,"
                  "m_isDelete             int                   );";

    QSqlQuery sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
    }

    return OptionalNotValue();
}

QString TaskPlanSqlPrivate::getParamPlaceholder(int count)
{
    QString res;
    for (int i = 0; i < count; ++i)
    {
        res += "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ,";
    }
    if (!res.isEmpty())
    {
        res.chop(1);
    }
    return res;
}

Optional<TaskPlanData> TaskPlanSqlPrivate::insertPlan(QSqlDatabase& db, const TaskPlanData& bak, bool isCreatUuid)
{
    using ResType = Optional<TaskPlanData>;
    auto planWorkTask = bak;

    QString sn;
    if (isCreatUuid)
    {
        auto snResult = TaskPlanSqlPrivate::createPlanID(db);
        if (!snResult)
        {
            return ResType(snResult.errorCode(), snResult.msg());
        }
        sn = snResult.value();
    }
    else
    {
        sn = bak.deviceWorkTask.m_plan_serial_number;
    }

    if (!planWorkTask.dataTransWorkTasks.isEmpty())
    {
        int i = 1;
        for (auto& sc : planWorkTask.dataTransWorkTasks)
        {
            sc.m_plan_serial_number = sn;
            /* 流水号  任务号 数传没有多目标的概念所以就是每个就是一个任务   */
            sc.m_uuid = QString("%1_%2").arg(sn).arg(i);
            ++i;
        }

        auto dtResult = insertPlan(db, planWorkTask.dataTransWorkTasks);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
    }

    if(planWorkTask.deviceWorkTask.m_uuid.isEmpty())
    {
        planWorkTask.deviceWorkTask.m_uuid = QString("%1_1").arg(sn);
    }

    planWorkTask.deviceWorkTask.m_plan_serial_number = sn;
    auto devResult = insertPlan(db, planWorkTask.deviceWorkTask);
    if (!devResult)
    {
        return ResType(devResult.errorCode(), devResult.msg());
    }
    return ResType(planWorkTask);
}
Optional<DataTranWorkTask> TaskPlanSqlPrivate::insertPlan(QSqlDatabase& db, const DataTranWorkTask& item)
{
    using ResType = Optional<DataTranWorkTask>;
    QString sql = QString("INSERT INTO DataTranWorkTask("
                          "m_uuid                ,"
                          "m_create_userid       ,"
                          "m_createTime          ,"
                          "m_task_start_time     ,"
                          "m_task_end_time       ,"
                          "m_transport_time      ,"
                          "m_task_commit_time    ,"
                          "m_plan_serial_number  ,"
                          "m_work_unit           ,"
                          "m_equipment_no        ,"
                          "m_task_total_num      ,"
                          "m_task_status         ,"
                          "m_working_mode        ,"
                          "m_task_code           ,"
                          "m_dt_transNum         ,"
                          "m_channelNum          ,"
                          "m_datatrans_center    ,"
                          "m_center_protocol     ,"
                          "m_datatrans_center_id ,"
                          "m_priority            ,"
                          "m_transport_protocols ,"
                          "m_ring_num            ,"
                          "m_source              ,"
                          "m_belongTarget        ,"
                          "m_isDelete            )"
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(sql);
    sqlQuery.bindValue(0, item.m_uuid);
    sqlQuery.bindValue(1, item.m_create_userid);
    sqlQuery.bindValue(2, Utility::dateTimeToStr(item.m_createTime));
    sqlQuery.bindValue(3, Utility::dateTimeToStr(item.m_task_start_time));
    sqlQuery.bindValue(4, Utility::dateTimeToStr(item.m_task_end_time));
    sqlQuery.bindValue(5, Utility::dateTimeToStr(item.m_transport_time));
    sqlQuery.bindValue(6, Utility::dateTimeToStr(item.m_task_commit_time));
    sqlQuery.bindValue(7, item.m_plan_serial_number);
    sqlQuery.bindValue(8, item.m_work_unit);
    sqlQuery.bindValue(9, item.m_equipment_no);
    sqlQuery.bindValue(10, item.m_task_total_num);
    sqlQuery.bindValue(11, static_cast<int>(item.m_task_status));
    sqlQuery.bindValue(12, static_cast<int>(item.m_working_mode));
    sqlQuery.bindValue(13, item.m_task_code);
    sqlQuery.bindValue(14, item.m_dt_transNum);
    sqlQuery.bindValue(15, item.m_channelNum);
    sqlQuery.bindValue(16, item.m_datatrans_center);
    sqlQuery.bindValue(17, static_cast<int>(item.m_center_protocol));
    sqlQuery.bindValue(18, item.m_datatrans_center_id);
    sqlQuery.bindValue(19, static_cast<int>(item.m_priority));
    sqlQuery.bindValue(20, item.m_transport_protocols);
    sqlQuery.bindValue(21, item.m_ring_num);
    sqlQuery.bindValue(22, static_cast<int>(item.m_source));
    sqlQuery.bindValue(23, item.m_belongTarget);
    sqlQuery.bindValue(24, 0); /* 是否删除标记为否 */

    sqlQuery.exec();

    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
}
Optional<DataTranWorkTaskList> TaskPlanSqlPrivate::insertPlan(QSqlDatabase& db, const DataTranWorkTaskList& items)
{
    using ResType = Optional<QList<DataTranWorkTask>>;

    QString sql = QString("INSERT INTO DataTranWorkTask("
                          "m_uuid                ,"
                          "m_create_userid       ,"
                          "m_createTime          ,"
                          "m_task_start_time     ,"
                          "m_task_end_time       ,"
                          "m_transport_time      ,"
                          "m_task_commit_time    ,"
                          "m_plan_serial_number  ,"
                          "m_work_unit           ,"
                          "m_equipment_no        ,"
                          "m_task_total_num      ,"
                          "m_task_status         ,"
                          "m_working_mode        ,"
                          "m_task_code           ,"
                          "m_dt_transNum         ,"
                          "m_channelNum          ,"
                          "m_datatrans_center    ,"
                          "m_center_protocol     ,"
                          "m_datatrans_center_id ,"
                          "m_priority            ,"
                          "m_transport_protocols ,"
                          "m_ring_num            ,"
                          "m_source              ,"
                          "m_belongTarget        ,"
                          "m_isDelete            )"
                          "VALUES %1;");

    sql = sql.arg(TaskPlanSqlPrivate::getParamPlaceholder(items.size()));

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(sql);
    int i = 0;
    for (auto& item : items)
    {
        auto tempNum = i * 25;
        sqlQuery.bindValue(0 + tempNum, item.m_uuid);
        sqlQuery.bindValue(1 + tempNum, item.m_create_userid);
        sqlQuery.bindValue(2 + tempNum, Utility::dateTimeToStr(item.m_createTime));
        sqlQuery.bindValue(3 + tempNum, Utility::dateTimeToStr(item.m_task_start_time));
        sqlQuery.bindValue(4 + tempNum, Utility::dateTimeToStr(item.m_task_end_time));
        sqlQuery.bindValue(5 + tempNum, Utility::dateTimeToStr(item.m_transport_time));
        sqlQuery.bindValue(6 + tempNum, Utility::dateTimeToStr(item.m_task_commit_time));
        sqlQuery.bindValue(7 + tempNum, item.m_plan_serial_number);
        sqlQuery.bindValue(8 + tempNum, item.m_work_unit);
        sqlQuery.bindValue(9 + tempNum, item.m_equipment_no);
        sqlQuery.bindValue(10 + tempNum, item.m_task_total_num);
        sqlQuery.bindValue(11 + tempNum, static_cast<int>(item.m_task_status));
        sqlQuery.bindValue(12 + tempNum, static_cast<int>(item.m_working_mode));
        sqlQuery.bindValue(13 + tempNum, item.m_task_code);
        sqlQuery.bindValue(14 + tempNum, item.m_dt_transNum);
        sqlQuery.bindValue(15 + tempNum, item.m_channelNum);
        sqlQuery.bindValue(16 + tempNum, item.m_datatrans_center);
        sqlQuery.bindValue(17 + tempNum, static_cast<int>(item.m_center_protocol));
        sqlQuery.bindValue(18 + tempNum, item.m_datatrans_center_id);
        sqlQuery.bindValue(19 + tempNum, static_cast<int>(item.m_priority));
        sqlQuery.bindValue(20 + tempNum, item.m_transport_protocols);
        sqlQuery.bindValue(21 + tempNum, item.m_ring_num);
        sqlQuery.bindValue(22 + tempNum, static_cast<int>(item.m_source));
        sqlQuery.bindValue(23 + tempNum, item.m_belongTarget);
        sqlQuery.bindValue(24 + tempNum, 0); /* 是否删除标记为否 */

        ++i;
    }

    sqlQuery.exec();

    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(items);
}
Optional<DeviceWorkTask> TaskPlanSqlPrivate::insertPlan(QSqlDatabase& db, const DeviceWorkTask& item)
{
    using ResType = Optional<DeviceWorkTask>;
    SqlGlobalData::updateItem(db, sqlGlobalDataPlanIDName, item.m_plan_serial_number.toUtf8());
    QString sql = QString("INSERT INTO DeviceWorkTask("
                          "m_uuid               ,"
                          "m_create_userid      ,"
                          "m_createTime         ,"
                          "m_preStartTime       ,"
                          "m_startTime          ,"
                          "m_endTime            ,"
                          "m_plan_serial_number ,"
                          "m_work_unit          ,"
                          "m_equipment_no       ,"
                          "m_scheduling_mode    ,"
                          "m_task_total_num     ,"
                          "m_is_zero            ,"
                          "m_calibration        ,"
                          "m_working_mode       ,"
                          "m_target_number      ,"
                          "m_lord_with_target   ,"
                          "m_tracking_order     ,"
                          "m_task_status        ,"
                          "m_source             ,"
                          "m_totalTargetMap     ,"
                          "m_isDelete           )"
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(sql);
    sqlQuery.bindValue(0, item.m_uuid);
    sqlQuery.bindValue(1, item.m_create_userid);
    sqlQuery.bindValue(2, Utility::dateTimeToStr(item.m_createTime));
    sqlQuery.bindValue(3, Utility::dateTimeToStr(item.m_preStartTime));
    sqlQuery.bindValue(4, Utility::dateTimeToStr(item.m_startTime));
    sqlQuery.bindValue(5, Utility::dateTimeToStr(item.m_endTime));
    sqlQuery.bindValue(6, item.m_plan_serial_number);
    sqlQuery.bindValue(7, item.m_work_unit);
    sqlQuery.bindValue(8, item.m_equipment_no);
    sqlQuery.bindValue(9, item.m_scheduling_mode);
    sqlQuery.bindValue(10, item.m_task_total_num);
    sqlQuery.bindValue(11, (int)item.m_is_zero);
    sqlQuery.bindValue(12, (int)item.m_calibration);
    sqlQuery.bindValue(13, item.m_working_mode);
    sqlQuery.bindValue(14, item.m_target_number);
    sqlQuery.bindValue(15, item.m_lord_with_target);
    sqlQuery.bindValue(16, item.m_tracking_order);
    sqlQuery.bindValue(17, static_cast<int>(item.m_task_status));
    sqlQuery.bindValue(18, static_cast<int>(item.m_source));

    QByteArray jsonData;
    jsonData << item.m_totalTargetMap;
    sqlQuery.bindValue(19, jsonData);

    sqlQuery.bindValue(20, 0); /* 是否删除标记为否 */

    sqlQuery.exec();

    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
}

Optional<bool> TaskPlanSqlPrivate::checkTaskIsUpdate(QSqlDatabase& db, const QString& uuid)
{
    using ResType = Optional<bool>;
    auto sql = QString("SELECT * FROM DeviceWorkTask WHERE m_uuid = '%1' ;").arg(uuid);

    auto sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }
    if (!sqlQuery.next())
    {
        return ResType(ErrorCode::NotFound, "未找到当前计划");
    }
    auto value = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
    /* 1.状态必须处于未运行才能更新 */
    return ResType(value.m_task_status == TaskPlanStatus::NoStart);
}

OptionalNotValue TaskPlanSqlPrivate::deletePlan(QSqlDatabase& db, const QString& deviceTaskID)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    auto idList = deviceTaskID.split("_");
    if (idList.size() != 2)
    {
        auto errMsg = QString("当前计划ID错误:%1").arg(deviceTaskID);
        return OptionalNotValue(ErrorCode::InvalidArgument, errMsg);
    }

    /* 先查出设备工作计划的数据  */
    /* 先查出设备工作计划的数据  */
    auto dtResult = queryPlanByID(db, deviceTaskID);
    if (!dtResult)
    {
        return OptionalNotValue(dtResult.errorCode(), dtResult.msg());
    }
    auto& deviceWorkTask = dtResult.value();

    /* 先删除数传工作计划 */
    QString sqlWhere;
    for (auto& item : deviceWorkTask.m_totalTargetMap)
    {
        /* 为空则当前目标没有数传 */
        if (item.m_dt_transNum.isEmpty())
        {
            continue;
        }

        sqlWhere += QString("m_dt_transNum = '%1' or ").arg(item.m_dt_transNum);
    }
    /* 为空是没有数传 */
    if (!sqlWhere.isEmpty())
    {
        /* 必须是or+空格 三个字符 */
        sqlWhere.chop(3);
        QString sql = "delete from DataTranWorkTask where m_uuid in "
                      "(select m_uuid from "
                      "(select * from DataTranWorkTask where m_uuid like '%1%') as tempTable1 "
                      "where %2);";
        sql = sql.arg(idList.at(0), sqlWhere);
        auto sqlQueryDT = db.exec(sql);

        auto lastError = sqlQueryDT.lastError();
        if (lastError.isValid())
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
        }
    }

    QString sql = "DELETE FROM DEVICEWORKTASK WHERE m_uuid = '%1';";
    sql = sql.arg(deviceTaskID);
    auto sqlQueryDev = db.exec(sql);

    auto lastError = sqlQueryDev.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
OptionalNotValue TaskPlanSqlPrivate::deletePlanMark(QSqlDatabase& db, const QString& deviceTaskID)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    auto idList = deviceTaskID.split("_");
    if (idList.size() != 2)
    {
        auto errMsg = QString("当前计划ID错误:%1").arg(deviceTaskID);
        return OptionalNotValue(ErrorCode::InvalidArgument, errMsg);
    }

    /* 先查出设备工作计划的数据  */
    /* 先查出设备工作计划的数据  */
    auto dtResult = queryPlanByID(db, deviceTaskID);
    if (!dtResult)
    {
        return OptionalNotValue(dtResult.errorCode(), dtResult.msg());
    }
    auto& deviceWorkTask = dtResult.value();

    /* 先删除数传工作计划 */
    QString sqlWhere;
    for (auto& item : deviceWorkTask.m_totalTargetMap)
    {
        /* 为空则当前目标没有数传 */
        if (item.m_dt_transNum.isEmpty())
        {
            continue;
        }

        sqlWhere += QString("m_dt_transNum = '%1' or ").arg(item.m_dt_transNum);
    }
    /* 为空是没有数传 */
    if (!sqlWhere.isEmpty())
    {
        /* 必须是or+空格 三个字符 */
        sqlWhere.chop(3);
        QString sql = "update DataTranWorkTask set m_isDelete = '1' where m_uuid in "
                      "(select m_uuid from "
                      "(select * from DataTranWorkTask where m_uuid like '%1%') as tempTable1 "
                      "where %2);";
        sql = sql.arg(idList.at(0), sqlWhere);
        auto sqlQueryDT = db.exec(sql);

        auto lastError = sqlQueryDT.lastError();
        if (lastError.isValid())
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
        }
    }

    QString sql = "update deviceworktask set m_isDelete = '1' where m_uuid = '%1';";
    sql = sql.arg(deviceTaskID);
    auto sqlQueryDev = db.exec(sql);

    auto lastError = sqlQueryDev.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}

Optional<DeviceWorkTask> TaskPlanSqlPrivate::queryPlanByID(QSqlDatabase& db, const QString& deviceTaskID)
{
    using ResType = Optional<DeviceWorkTask>;
    QString sql = QString("SELECT * FROM DeviceWorkTask WHERE m_uuid = '%1';").arg(deviceTaskID);
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }
    if (!sqlQuery.next())
    {
        auto errMsg = QString("未找到当前ID:%1对应的任务计划").arg(deviceTaskID);
        return ResType(ErrorCode::NotFound, errMsg);
    }
    auto result = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
    return ResType(result);
}
Optional<DeviceWorkTask> TaskPlanSqlPrivate::queryPlanByIDMark(QSqlDatabase& db, const QString& deviceTaskID)
{
    using ResType = Optional<DeviceWorkTask>;
    QString sql = "select * from DeviceWorkTask where m_uuid in "
                  "(select m_uuid from "
                  "(select * from DeviceWorkTask where m_uuid = '%1') as tempTable1 "
                  "where m_isDelete = '0' );";

    sql = sql.arg(deviceTaskID);
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }
    if (!sqlQuery.next())
    {
        auto errMsg = QString("未找到当前ID:%1对应的任务计划").arg(deviceTaskID);
        return ResType(ErrorCode::NotFound, errMsg);
    }
    auto result = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
    return ResType(result);
}

Optional<DataTranWorkTaskList> TaskPlanSqlPrivate::queryPlanByDev(QSqlDatabase& db, const DeviceWorkTask& deviceWorkTask)
{
    using ResType = Optional<DataTranWorkTaskList>;
    QString sqlWhere;
    for (auto& item : deviceWorkTask.m_totalTargetMap)
    {
        /* 为空则当前目标没有数传 */
        if (item.m_dt_transNum.isEmpty())
        {
            continue;
        }

        sqlWhere += QString("m_dt_transNum = '%1' or ").arg(item.m_dt_transNum);
    }
    /* 没有数传 */
    if (sqlWhere.isEmpty())
    {
        return ResType(DataTranWorkTaskList());
    }
    /* 必须是or+空格 三个字符 */
    sqlWhere.chop(3);
    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QString sql = "select * from DataTranWorkTask where m_uuid in "
                  "(select m_uuid from "
                  "(select * from DataTranWorkTask where m_uuid like '%1%') as tempTable1 "
                  "where %2);";
    sql = sql.arg(deviceWorkTask.m_plan_serial_number, sqlWhere);

    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DataTranWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        result << data;
    }
    return ResType(result);
}
Optional<DataTranWorkTaskList> TaskPlanSqlPrivate::queryPlanByDevMark(QSqlDatabase& db, const DeviceWorkTask& deviceWorkTask)
{
    using ResType = Optional<DataTranWorkTaskList>;
    QString sqlWhere;
    for (auto& item : deviceWorkTask.m_totalTargetMap)
    {
        /* 为空则当前目标没有数传 */
        if (item.m_dt_transNum.isEmpty())
        {
            continue;
        }

        sqlWhere += QString("m_dt_transNum = '%1' or ").arg(item.m_dt_transNum);
    }
    /* 没有数传 */
    if (sqlWhere.isEmpty())
    {
        return ResType(DataTranWorkTaskList());
    }
    /* 必须是or+空格 三个字符 */
    sqlWhere.chop(3);
    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QString sql = "select * from DataTranWorkTask where m_uuid in "
                  "(select m_uuid from "
                  "(select * from DataTranWorkTask where m_uuid like '%1%') as tempTable1 "
                  "where m_isDelete = '0' and ( %2 ) );";
    sql = sql.arg(deviceWorkTask.m_plan_serial_number, sqlWhere);

    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DataTranWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        result << data;
    }
    return ResType(result);
}

Optional<DeviceWorkTaskList> TaskPlanSqlPrivate::queryPlanBySN(QSqlDatabase& db, const QString& sn)
{
    using ResType = Optional<DeviceWorkTaskList>;
    QString sql = QString("SELECT * FROM DeviceWorkTask WHERE m_uuid like '%1%';").arg(sn);
    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DeviceWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
        result << data;
    }
    return ResType(result);
}
Optional<DeviceWorkTaskList> TaskPlanSqlPrivate::queryPlanBySNMark(QSqlDatabase& db, const QString& sn)
{
    using ResType = Optional<DeviceWorkTaskList>;
    QString sql = "select * from DeviceWorkTask where m_uuid in "
                  "(select m_uuid from "
                  "(select * from DeviceWorkTask where m_uuid like '%1%') as tempTable1 "
                  "where m_isDelete = '0' );";
    sql = sql.arg(sn);

    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DeviceWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
        result << data;
    }
    return ResType(result);
}

Optional<DataTranWorkTaskList> TaskPlanSqlPrivate::queryPlanBySN(QSqlDatabase& db, const QString& sn, const QString& traceNum)
{
    using ResType = Optional<DataTranWorkTaskList>;
    /* 为空则当前目标没有数传 */
    QString sql;
    if (traceNum.isEmpty())
    {
        sql = QString("select * from DataTranWorkTask where m_uuid like '%1%'").arg(sn);
    }
    else
    {
        sql = "select * from DataTranWorkTask where m_uuid in "
              "(select m_uuid from "
              "(select * from DataTranWorkTask where m_uuid like '%1%') as tempTable1 "
              "where m_dt_transNum='%2');";
        sql = sql.arg(sn, traceNum);
    }
    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DataTranWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        result << data;
    }
    return ResType(result);
}
Optional<DataTranWorkTaskList> TaskPlanSqlPrivate::queryPlanBySNMark(QSqlDatabase& db, const QString& sn, const QString& traceNum)
{
    using ResType = Optional<DataTranWorkTaskList>;
    /* 为空则当前目标没有数传 */
    QString sql;
    if (traceNum.isEmpty())
    {
        sql = "select * from DataTranWorkTask where m_uuid in "
              "(select m_uuid from "
              "(select * from DataTranWorkTask where m_uuid like '%1%' ) as tempTable1 "
              "where m_isDelete = '0' );";
        sql = sql.arg(sn);
    }
    else
    {
        sql = "select * from DataTranWorkTask where m_uuid in "
              "(select m_uuid from "
              "(select * from DataTranWorkTask where m_uuid like '%1%' ) as tempTable1 "
              "where m_isDelete = '0' and m_dt_transNum= '%2' );";
        sql = sql.arg(sn, traceNum);
    }
    /*
     * 设备工作计划 m_uuid = m_plan_serial_number_任务号
     * 数传工作计划 m_uuid = m_plan_serial_number_任务号
     *
     * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
     */
    QSqlQuery sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    DataTranWorkTaskList result;
    while (sqlQuery.next())
    {
        auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        result << data;
    }
    return ResType(result);
}

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
OptionalNotValue TaskPlanSql::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        auto res = TaskPlanSqlPrivate::initDevicePlanDB(db.value());
        if (!res)
        {
            return res;
        }

        res = SqlGlobalData::initDB(db.value());
        if (!res)
        {
            return res;
        }
        SqlGlobalData::updateItemNotExist(db.value(), sqlGlobalDataPlanIDName);
        return TaskPlanSqlPrivate::initDTPlanDB(db.value());
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryAllDTPlan()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT  * FROM DataTranWorkTask ";
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<DataTranWorkTask> items;
        while (sqlQuery.next())
        {
            items << TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        }

        return ResType(items);
    }
}
Optional<DeviceWorkTaskList> TaskPlanSql::queryAllDevPlan()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT  * FROM DeviceWorkTask ;";
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<DeviceWorkTask> items;
        while (sqlQuery.next())
        {
            items << TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
        }

        return ResType(items);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryAllDTPlanMark()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT  * FROM DataTranWorkTask where m_isDelete = '0' ";
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<DataTranWorkTask> items;
        while (sqlQuery.next())
        {
            items << TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
        }

        return ResType(items);
    }
}
Optional<DeviceWorkTaskList> TaskPlanSql::queryAllDevPlanMark()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT  * FROM DeviceWorkTask where m_isDelete = '0' ;";
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<DeviceWorkTask> items;
        while (sqlQuery.next())
        {
            items << TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
        }

        return ResType(items);
    }
}

Optional<TaskPlanData> TaskPlanSql::insertPlan(const TaskPlanData& bak)
{
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::insertPlan(db.value(), bak, true);
    }
}
Optional<DataTranWorkTask> TaskPlanSql::insertPlan(const DataTranWorkTask& item)
{
    using ResType = Optional<DataTranWorkTask>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::insertPlan(db.value(), item);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::insertPlan(const DataTranWorkTaskList& item)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::insertPlan(db.value(), item);
    }
}
Optional<DeviceWorkTask> TaskPlanSql::insertPlan(const DeviceWorkTask& item)
{
    using ResType = Optional<DeviceWorkTask>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        return TaskPlanSqlPrivate::insertPlan(db.value(), item);
    }
}

Optional<TaskPlanData> TaskPlanSql::updatePlan(const TaskPlanData& bak)
{
    using ResType = Optional<TaskPlanData>;
    auto planWorkTask = bak;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        /* 更新计划需要检查原计划是否可以更新 */
        auto result = TaskPlanSqlPrivate::checkTaskIsUpdate(db.value(), planWorkTask.deviceWorkTask.m_uuid);
        if (!result)
        {
            return ResType(result.errorCode(), result.msg());
        }
        if (!result.value())
        {
            return ResType(ErrorCode::NotSupported, "当前状态不是未运行,不支持更新编辑");
        }
        /* 这里直接删除原来的
         * 然后重新插入
         * 因为编辑可能会改变数传计划的数量
         */
        auto deleteResult = TaskPlanSqlPrivate::deletePlan(db.value(), planWorkTask.deviceWorkTask.m_uuid);
        if (!deleteResult)
        {
            return ResType(deleteResult.errorCode(), deleteResult.msg());
        }
        return TaskPlanSqlPrivate::insertPlan(db.value(), planWorkTask, false);
    }
}

OptionalNotValue TaskPlanSql::updataTaskStatus(const QString& deviceTaskID, TaskPlanStatus status)
{
    /*
     * 根据流水号更新设备工作计划的状态
     * 更新设备工作计划的状态
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    auto idList = deviceTaskID.split("_");
    if (idList.size() != 2)
    {
        auto errMsg = QString("当前计划ID错误:%1").arg(deviceTaskID);
        return OptionalNotValue(ErrorCode::InvalidArgument, errMsg);
    }

    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        /* 先查出设备工作计划的数据  */
        auto dtResult = TaskPlanSqlPrivate::queryPlanByID(db.value(), deviceTaskID);
        if (!dtResult)
        {
            return OptionalNotValue(dtResult.errorCode(), dtResult.msg());
        }

        /* 更新设备工作计划状态 */
        auto sql = QString("UPDATE DeviceWorkTask SET m_task_status = '%1' WHERE m_uuid = '%2';");
        sql = sql.arg(int(status)).arg(deviceTaskID);
        auto sqlQueryDev = db->exec(sql);

        auto lastError = sqlQueryDev.lastError();
        if (lastError.isValid())
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
        }

        /* 更新数传工作计划状态 */
        auto& deviceWorkTask = dtResult.value();
        QString sqlWhere;
        for (auto& item : deviceWorkTask.m_totalTargetMap)
        {
            /* 为空则当前目标没有数传 */
            if (item.m_dt_transNum.isEmpty())
            {
                continue;
            }

            sqlWhere += QString("m_dt_transNum = '%1' or ").arg(item.m_dt_transNum);
        }
        /* 没有数传计划 */
        if (sqlWhere.isEmpty())
        {
            return OptionalNotValue();
        }
        /* 必须是or+空格 三个字符 */
        sqlWhere.chop(3); /* 去掉末尾的or */

        sql = "update datatranworktask SET m_task_status = '%1' where m_uuid in "
              "(select m_uuid from "
              "(select * from DataTranWorkTask where m_uuid like '%2%') as tempTable1 "
              "where %3);";
        sql = sql.arg(int(status)).arg(idList.at(0), sqlWhere);
        auto sqlQueryDT = db->exec(sql);
        lastError = sqlQueryDT.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

OptionalNotValue TaskPlanSql::deletePlan(const QString& deviceTaskID)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */

    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::deletePlan(db.value(), deviceTaskID);
    }
}
OptionalNotValue TaskPlanSql::deleteDTPlan(const QString& sn)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */

    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        auto sql = QString("delete from DataTranWorkTask where m_uuid like '%1%'").arg(sn);
        auto sqlQueryDT = db->exec(sql);

        auto lastError = sqlQueryDT.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
OptionalNotValue TaskPlanSql::deleteDevPlan(const QString& sn)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        auto sql = QString("delete from DeviceWorkTask where m_uuid like '%1%'").arg(sn);
        auto sqlQueryDev = db->exec(sql);

        auto lastError = sqlQueryDev.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
OptionalNotValue TaskPlanSql::deletePlanMark(const QString& deviceTaskID)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     *
     * 20210622 wp 增加不真正删除任务
     * 只标记为删除,不真正删除数据
     * 问题是任务会越来越多,此问题不是此方法考虑的
     */

    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::deletePlanMark(db.value(), deviceTaskID);
    }
}
OptionalNotValue TaskPlanSql::deleteDTPlanMark(const QString& sn)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     *
     * 只是标记为删除,数据依然存在
     */

    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        auto sql = QString("update DataTranWorkTask set m_isDelete = '1' where m_uuid like '%1%' ;").arg(sn);
        auto sqlQueryDT = db->exec(sql);

        auto lastError = sqlQueryDT.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
OptionalNotValue TaskPlanSql::deleteDevPlanMark(const QString& sn)
{
    /*
     * 一定要先删除数传,如果先删除设备的,如果设备删除成功数传删除失败,就再也无法索引删除数传的了
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     *
     * 只是标记为删除,数据依然存在
     */
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        auto sql = QString("update DeviceWorkTask set m_isDelete = '1' where m_uuid like '%1%' ;").arg(sn);
        auto sqlQueryDev = db->exec(sql);

        auto lastError = sqlQueryDev.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<TaskPlanData> TaskPlanSql::queryPlanByID(const QString& deviceTaskID)
{
    /*
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 查询设备工作计划 */
        auto devResult = TaskPlanSqlPrivate::queryPlanByID(db.value(), deviceTaskID);
        if (!devResult)
        {
            return ResType(devResult.errorCode(), devResult.msg());
        }

        TaskPlanData result;
        result.deviceWorkTask = devResult.value();

        auto dtResult = TaskPlanSqlPrivate::queryPlanByDev(db.value(), result.deviceWorkTask);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
        result.dataTransWorkTasks = dtResult.value();
        return ResType(result);
    }
}
Optional<TaskPlanData> TaskPlanSql::queryPlanByIDMark(const QString& deviceTaskID)
{
    /*
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 查询设备工作计划 */
        auto devResult = TaskPlanSqlPrivate::queryPlanByIDMark(db.value(), deviceTaskID);
        if (!devResult)
        {
            return ResType(devResult.errorCode(), devResult.msg());
        }

        TaskPlanData result;
        result.deviceWorkTask = devResult.value();

        auto dtResult = TaskPlanSqlPrivate::queryPlanByDevMark(db.value(), result.deviceWorkTask);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
        result.dataTransWorkTasks = dtResult.value();
        return ResType(result);
    }
}

Optional<TaskPlanData> TaskPlanSql::queryPlanBySN(const QString& sn)
{
    /*
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 查询设备工作计划 */
        auto devResult = TaskPlanSqlPrivate::queryPlanBySN(db.value(), sn);
        if (!devResult)
        {
            return ResType(devResult.errorCode(), devResult.msg());
        }

        /* 查询不唯一 */
        auto size = devResult->size();
        if (size > 1)
        {
            auto errMsg = QString("当前流水号对应%1个任务").arg(size);
            return ResType(ErrorCode::InvalidData, errMsg);
        }

        TaskPlanData result;
        if (!devResult->isEmpty())
        {
            result.deviceWorkTask = devResult->first();
        }
        auto dtResult = TaskPlanSqlPrivate::queryPlanByDev(db.value(), result.deviceWorkTask);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
        result.dataTransWorkTasks = dtResult.value();
        return ResType(result);
    }
}
Optional<TaskPlanData> TaskPlanSql::queryPlanBySNMark(const QString& sn)
{
    /*
     *
     * 这里的设备工作计划uuid就是流水号_任务号
     * 数传计划需要先根据流水号查出所有数传计划 然后根据设备计划的跟踪号来更新数传计划
     *
     * 手动添加的都任务号都是1
     * 只有中心下计划的，才会出现任务为多个的情况
     */
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 查询设备工作计划 */
        auto devResult = TaskPlanSqlPrivate::queryPlanBySNMark(db.value(), sn);
        if (!devResult)
        {
            return ResType(devResult.errorCode(), devResult.msg());
        }

        /* 查询不唯一 */
        auto size = devResult->size();
        if (size > 1)
        {
            auto errMsg = QString("当前流水号对应%1个任务").arg(size);
            return ResType(ErrorCode::InvalidData, errMsg);
        }

        TaskPlanData result;
        if (!devResult->isEmpty())
        {
            result.deviceWorkTask = devResult->first();
        }
        auto dtResult = TaskPlanSqlPrivate::queryPlanByDevMark(db.value(), result.deviceWorkTask);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
        result.dataTransWorkTasks = dtResult.value();
        return ResType(result);
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryPlanByDev(const DeviceWorkTask& deviceWorkTask)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        return TaskPlanSqlPrivate::queryPlanByDev(db.value(), deviceWorkTask);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryPlanByDevMark(const DeviceWorkTask& deviceWorkTask)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        return TaskPlanSqlPrivate::queryPlanByDevMark(db.value(), deviceWorkTask);
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryPlanBySN(const QString& sn, const QString& traceNum)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::queryPlanBySN(db.value(), sn, traceNum);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryPlanBySNMark(const QString& sn, const QString& traceNum)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::queryPlanBySNMark(db.value(), sn, traceNum);
    }
}

Optional<DeviceWorkTaskList> TaskPlanSql::queryHistoryDevPlan(const QDate& startDate, const QDate& endDate)
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 加一天因为BETWEEN不包括末尾的天数也就是小于 但是这里我需要当前的数据,所以加一 */
        auto sql = QString("SELECT  * FROM DeviceWorkTask WHERE m_uuid BETWEEN '%1' AND '%2';")
                       .arg(startDate.toString(snDateFormat), endDate.addDays(1).toString(snDateFormat));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DeviceWorkTaskList> TaskPlanSql::queryHistoryDevPlanMark(const QDate& startDate, const QDate& endDate)
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 加一天因为BETWEEN不包括末尾的天数也就是小于 但是这里我需要当前的数据,所以加一 */
        QString sql = "select * from DeviceWorkTask where m_uuid in "
                      "(select m_uuid from "
                      "(select * from DeviceWorkTask where m_uuid between '%1' and '%2' ) as tempTable1 "
                      "where m_isDelete = '0' );";
        sql = sql.arg(startDate.toString(snDateFormat), endDate.addDays(1).toString(snDateFormat));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryHistoryDTPlan(const QDate& startDate, const QDate& endDate)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 加一天因为BETWEEN不包括末尾的天数也就是小于 但是这里我需要当前的数据,所以加一 */
        auto sql = QString("SELECT  * FROM DataTranWorkTask WHERE m_uuid BETWEEN '%1' AND '%2';")
                       .arg(startDate.toString(snDateFormat), endDate.addDays(1).toString(snDateFormat));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryHistoryDTPlanMark(const QDate& startDate, const QDate& endDate)
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 加一天因为BETWEEN不包括末尾的天数也就是小于 但是这里我需要当前的数据,所以加一 */
        QString sql = "select * from DataTranWorkTask where m_uuid in "
                      "(select m_uuid from "
                      "(select * from DataTranWorkTask where m_uuid between '%1' and '%2' ) as tempTable1 "
                      "where m_isDelete = '0' );";
        sql = sql.arg(startDate.toString(snDateFormat), endDate.addDays(1).toString(snDateFormat));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DeviceWorkTaskConditionQueryACK> TaskPlanSql::getHistoryDevPlan(const DeviceWorkTaskConditionQuery& item)
{
    using ResType = Optional<DeviceWorkTaskConditionQueryACK>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        DeviceWorkTaskConditionQueryACK ack;
        QSqlQuery sqlQuery(db.value());
        int totalCount = 0;
        {
            QString sqlTemplate;
            if (item.taskStatus == -1)
            {
                //                sqlTemplate = "SELECT  * FROM DeviceWorkTask WHERE m_createTime BETWEEN '%1' AND ?;";
                //                sqlQuery.prepare(sqlTemplate);
                //                sqlQuery.bindValue(0, Utility::timeToStr(item.beginTime));
                //                sqlQuery.bindValue(1, Utility::timeToStr(item.endTime));

                sqlTemplate = "SELECT  * FROM DeviceWorkTask WHERE m_createTime BETWEEN '%1' AND '%2' order by m_createTime asc;";
                sqlTemplate = sqlTemplate.arg(Utility::dateTimeToStr(item.beginDate)).arg(Utility::dateTimeToStr(item.endDate));
            }
            else
            {
                sqlTemplate =
                    "SELECT  * FROM DeviceWorkTask WHERE m_createTime BETWEEN '%1' AND '%2' AND m_task_status = '%3' order by m_createTime asc;";
                sqlTemplate = sqlTemplate.arg(Utility::dateTimeToStr(item.beginDate)).arg(Utility::dateTimeToStr(item.endDate)).arg(item.taskStatus);
            }

            sqlQuery = db->exec(sqlTemplate);
            qDebug() << sqlTemplate;

            while (sqlQuery.next())
            {
                totalCount++;
            }
        }
        {
            QString sqlTemplate;
            if (item.taskStatus == -1)
            {
                sqlTemplate = "SELECT * FROM DeviceWorkTask WHERE m_createTime BETWEEN '%1' AND '%2' order by m_createTime limit %3,%4;";
                sqlTemplate = sqlTemplate.arg(Utility::dateTimeToStr(item.beginDate))
                                  .arg(Utility::dateTimeToStr(item.endDate))
                                  .arg((item.currentPage - 1) * item.pageSize)
                                  .arg(item.pageSize);
            }
            else
            {
                sqlTemplate = "SELECT * FROM DeviceWorkTask WHERE m_createTime BETWEEN '%1' AND '%2'  AND m_task_status = %3 order by m_createTime "
                              "limit %4,%5;";
                sqlTemplate = sqlTemplate.arg(Utility::dateTimeToStr(item.beginDate))
                                  .arg(Utility::dateTimeToStr(item.endDate))
                                  .arg(item.taskStatus)
                                  .arg((item.currentPage - 1) * item.pageSize)
                                  .arg(item.pageSize);
            }
            //            qDebug() << sqlTemplate;

            sqlQuery.exec(sqlTemplate);
            qDebug() << sqlTemplate;

            auto lastError = sqlQuery.lastError();
            if (lastError.isValid())
            {
                return ResType(db.errorCode(), db.msg());
            }

            QList<DeviceWorkTask> items;

            while (sqlQuery.next())
            {
                //查询数据行
                items << TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            }

            ack.totalCount = totalCount;
            ack.currentPage = item.currentPage;
            ack.deviceWorkTasks = items;
            return ResType(ack);
        }
    }
}

Optional<DeviceWorkTaskList> TaskPlanSql::queryRunningOrNoStartDevPlan()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where (m_task_status ='%2' or m_task_status = '%3')";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                  .arg(int(TaskPlanStatus::NoStart))
                  .arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DeviceWorkTaskList> TaskPlanSql::queryRunningOrNoStartDevPlanMark()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where m_isDelete = '0' and (m_task_status ='%2' or m_task_status = '%3')";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                  .arg(int(TaskPlanStatus::NoStart))
                  .arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DeviceWorkTaskList> TaskPlanSql::queryNoStartDevPlan()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where (m_task_status ='%2' )";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DeviceWorkTaskList> TaskPlanSql::queryNoStartDevPlanMark()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where m_isDelete = '0' and m_task_status ='%2' ";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryRunningOrNoStartDTPlan()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM DataTranWorkTask where m_uuid >= '%1' ) as temptable "
                      "where (m_task_status ='%2' or m_task_status = '%3')  ";

        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                  .arg(int(TaskPlanStatus::NoStart))
                  .arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryRunningOrNoStartDTPlanMark()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM DataTranWorkTask where m_uuid >= '%1' ) as temptable "
                      "where m_isDelete = '0' and (m_task_status ='%2' or m_task_status = '%3')  ";

        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                  .arg(int(TaskPlanStatus::NoStart))
                  .arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DeviceWorkTaskList> TaskPlanSql::queryRunningDevPlan()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where (m_task_status ='%2')";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DeviceWorkTaskList> TaskPlanSql::queryRunningDevPlanMark()
{
    using ResType = Optional<DeviceWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM deviceworktask where m_uuid >= '%1' ) as temptable "
                      "where m_isDelete = '0' and m_task_status ='%2' ";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::Running));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DeviceWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<DeviceWorkTask> TaskPlanSql::queryNextNewDevPlan()
{
    using ResType = Optional<DeviceWorkTask>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT * FROM deviceworktask WHERE m_isDelete = '0' AND m_task_status ='%1' ORDER BY  m_preStartTime LIMIT 1;";
        sql = sql.arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        if (!sqlQuery.next())
        {
            return ResType(ErrorCode::InvalidData, "未查询到需要开始的任务计划");
        }

        DeviceWorkTask result;
        result = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);

        return ResType(result);
    }
}
Optional<TaskPlanData> TaskPlanSql::queryNextNewPlan()
{
    using ResType = Optional<TaskPlanData>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "SELECT * FROM deviceworktask WHERE m_isDelete = '0' AND m_task_status ='%1' ORDER BY  m_preStartTime LIMIT 1;";
        sql = sql.arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        if (!sqlQuery.next())
        {
            return ResType(ErrorCode::InvalidData, "未查询到需要开始的任务计划");
        }

        TaskPlanData result;
        result.deviceWorkTask = TaskPlanSqlPrivate::sqlQueryToDevData(sqlQuery);

        auto dtResult = TaskPlanSqlPrivate::queryPlanByDevMark(db.value(), result.deviceWorkTask);
        if (!dtResult)
        {
            return ResType(dtResult.errorCode(), dtResult.msg());
        }
        result.dataTransWorkTasks = dtResult.value();
        return ResType(result);
    }
}

Optional<DataTranWorkTaskList> TaskPlanSql::queryNoStartDTPlan()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM DataTranWorkTask where m_uuid >= '%1' ) as temptable "
                      "where m_task_status ='%2'";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}
Optional<DataTranWorkTaskList> TaskPlanSql::queryNoStartDTPlanMark()
{
    using ResType = Optional<DataTranWorkTaskList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "select * from "
                      "(SELECT * FROM DataTranWorkTask where m_uuid >= '%1' ) as temptable "
                      "where m_isDelete = '0' and m_task_status ='%2'";
        sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat)).arg(int(TaskPlanStatus::NoStart));

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTranWorkTaskList result;
        while (sqlQuery.next())
        {
            auto data = TaskPlanSqlPrivate::sqlQueryToDTData(sqlQuery);
            result << data;
        }
        return ResType(result);
    }
}

Optional<QString> TaskPlanSql::createPlanID()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return Optional<QString>(db.errorCode(), db.msg());
        }
        return TaskPlanSqlPrivate::createPlanID(db.value());
    }
}

bool TaskPlanSql::dtPlanIsRunning(const QString& sn)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return false;
        }

        QString sql = "select m_uuid from "
                      "(select * from DataTranWorkTask where m_uuid like '%1%' ) as temptable "
                      "where m_task_status ='%2' and m_plan_serial_number = '%1' ";

        sql = sql.arg(sn).arg(int(TaskPlanStatus::Running));

        QSqlQuery sqlQuery(db.value());
        /* 执行错误,认为存在了,不让插入 */
        if (!sqlQuery.exec(sql))
        {
            return false;
        }
        return sqlQuery.next();
    }
}

bool TaskPlanSql::devPlanIsRunning(const QString& sn)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return false;
        }

        QString sql = "select m_uuid from "
                      "(select * from deviceworktask where m_uuid like '%1%' ) as temptable "
                      "where m_task_status ='%2' and m_plan_serial_number = '%1' ";

        sql = sql.arg(sn).arg(int(TaskPlanStatus::Running));

        QSqlQuery sqlQuery(db.value());
        /* 执行错误,认为存在了,不让插入 */
        if (!sqlQuery.exec(sql))
        {
            return false;
        }
        return sqlQuery.next();
    }
}
void TaskPlanSql::resetPlan()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return;
        }

        /* 设备工作计划清理 */
        QString sql = "update deviceworktask SET m_task_status = '%1' where m_uuid in "
              "(select m_uuid from "
              "(SELECT * FROM deviceworktask where m_uuid < '%2' ) as temptable "
              "where (m_task_status = '%3' or m_task_status = '%4'))";
        sql = sql.arg(int(TaskPlanStatus::Error))
                .arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                .arg(int(TaskPlanStatus::NoStart))
                .arg(int(TaskPlanStatus::Running));
        db->exec(sql);

        /* 数传工作计划 */
        sql = "update DataTranWorkTask SET m_task_status = '%1' where m_uuid in "
              "(select m_uuid from "
              "(SELECT * FROM DataTranWorkTask where m_uuid < '%2' ) as temptable "
              "where (m_task_status = '%3' or m_task_status = '%4'))";
        sql = sql.arg(int(TaskPlanStatus::Error))
                .arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                .arg(int(TaskPlanStatus::NoStart))
                .arg(int(TaskPlanStatus::Running));
        db->exec(sql);
    }
}
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

OptionalNotValue TaskPlanSql::initAutoTaskLogDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists AutoTaskLog(Id INT PRIMARY KEY AUTO_INCREMENT , createTime bigint,uuid text,taskCode text, level "
                      "int,userID text,context blob, serialNumber text);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "自动化运行日志数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<QList<AutoTaskLogData>> TaskPlanSql::queryAutoTaskLog(const QString& uuid, quint64 startTime, quint64 endTime)
{
    using ResType = Optional<QList<AutoTaskLogData>>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sqlTemplate = "SELECT * FROM AutoTaskLog where uuid = '%1' and createTime BETWEEN '%2' AND '%3' order by createTime;";
        QString sql = sqlTemplate.arg(uuid).arg(startTime).arg(endTime);
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<AutoTaskLogData> items;
        while (sqlQuery.next())
        {
            AutoTaskLogData data;
            //查询数据行
            QSqlRecord recordData = sqlQuery.record();
            data.uuid = recordData.field("uuid").value().toString();
            data.taskCode = recordData.field("taskCode").value().toString();
            data.serialNumber = recordData.field("serialNumber").value().toInt();
            data.createTime = recordData.field("createTime").value().toString();
            data.level = static_cast<AutoTaskLogLevel>(recordData.field("level").value().toInt());
            data.userID = recordData.field("userID").value().toString();
            data.context = recordData.field("context").value().toString();
            items.append(data);
        }

        return ResType(items);
    }
}

Optional<bool> TaskPlanSql::insertAutoTaskLog(const QList<AutoTaskLogData>& autoTaskLogList)
{
    using ResType = Optional<bool>;
    AutoCloseDBHelper closeHelper;
    {
        bool result = false;
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        bool flag = db.value().transaction();
        if (!flag)
        {
            qDebug() << "不支持事物";
            return ResType(result);
        }

        for (auto& item : autoTaskLogList)
        {
            QSqlQuery sqlQuery(db.value());
            QString sqlTemplate =
                "INSERT INTO AutoTaskLog (uuid, taskCode, serialNumber, createTime, level, userID, context) VALUES (?,?,?,?,?,?,?);";

            sqlQuery.prepare(sqlTemplate);
            sqlQuery.bindValue(0, item.uuid);
            sqlQuery.bindValue(1, item.taskCode);
            sqlQuery.bindValue(2, item.serialNumber);
            sqlQuery.bindValue(3, item.createTime);
            sqlQuery.bindValue(4, static_cast<int>(item.level));
            sqlQuery.bindValue(5, item.userID);
            sqlQuery.bindValue(6, item.context.toUtf8());
            sqlQuery.exec();
        }
        db.value().commit();
        result = true;
        return ResType(result);
    }
}
Optional<bool> TaskPlanSql::insertAutoTaskLog(const AutoTaskLogData& autoTaskLog)
{
    using ResType = Optional<bool>;
    AutoCloseDBHelper closeHelper;
    {
        bool result = false;
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QSqlQuery sqlQuery(db.value());
        QString sqlTemplate = "INSERT INTO AutoTaskLog (uuid, taskCode, serialNumber, createTime, level, userID, context) VALUES (?,?,?,?,?,?,?);";

        sqlQuery.prepare(sqlTemplate);
        sqlQuery.bindValue(0, autoTaskLog.uuid);
        sqlQuery.bindValue(1, autoTaskLog.taskCode);
        sqlQuery.bindValue(2, autoTaskLog.serialNumber);
        sqlQuery.bindValue(3, autoTaskLog.createTime);
        sqlQuery.bindValue(4, static_cast<int>(autoTaskLog.level));
        sqlQuery.bindValue(5, autoTaskLog.userID);
        sqlQuery.bindValue(6, autoTaskLog.context.toUtf8());
        sqlQuery.exec();
        result = true;
        return ResType(result);
    }
}

bool TaskPlanSql::checkTask(const QDateTime& startDateTime, const QDateTime& endDateTime, const QString& exclude)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return false;
        }

        QString sql;
        if (exclude.isEmpty())
        {
            sql = "select * from "
                  "(select * from "
                  "(select * from deviceworktask where m_uuid >= '%1' ) as temptable "
                  "where m_isDelete = '0' and (m_task_status ='%2' or m_task_status = '%3') ) as temp2table "
                  "where ('%4' between m_preStartTime AND m_endTime) or ('%5' between m_preStartTime AND m_endTime) ";

            sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat))
                      .arg(int(TaskPlanStatus::Running))
                      .arg(int(TaskPlanStatus::NoStart))
                      .arg(Utility::dateTimeToStr(startDateTime), Utility::dateTimeToStr(endDateTime));
        }
        else
        {
            sql = "select * from "
                  "(select * from "
                  "(select * from deviceworktask where m_uuid >= '%1' and m_uuid != '%2' ) as temptable "
                  "where m_isDelete = '0' and (m_task_status ='%3' or m_task_status = '%4') ) as temp2table "
                  "where ('%5' between m_preStartTime AND m_endTime) or ('%6' between m_preStartTime AND m_endTime) ";

            sql = sql.arg(GlobalData::currentDate().addDays(dayRetrogression).toString(snDateFormat), exclude)
                      .arg(int(TaskPlanStatus::Running))
                      .arg(int(TaskPlanStatus::NoStart))
                      .arg(Utility::dateTimeToStr(startDateTime), Utility::dateTimeToStr(endDateTime));
        }

        // 查询是否存在状态为正在运行或者未开始的任务时间冲突
        //        QString sqlTemplate = QString("SELECT * FROM ("
        //                                      "SELECT * FROM deviceworktask WHERE m_task_status ='%1' OR m_task_status ='%2') AS a1 "
        //                                      "WHERE ('%3' BETWEEN m_preStartTime AND m_endTime)"
        //                                      "OR ('%4' BETWEEN m_preStartTime AND m_endTime)")
        //                                  .arg(int(TaskPlanStatus::Running))
        //                                  .arg(int(TaskPlanStatus::NoStart))
        //                                  .arg(startDateTime.toString("yyyy-MM-dd HH:mm:ss"))
        //                                  .arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss"));

        // qWarning() << sqlTemplate;
        QSqlQuery sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return false;
        }

        while (sqlQuery.next())
        {
            return true;
        }
        return false;
    }
}
