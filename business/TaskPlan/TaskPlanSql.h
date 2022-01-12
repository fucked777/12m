#ifndef TASKPLANSQL_H
#define TASKPLANSQL_H

#include "Utility.h"
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QSqlDatabase>

enum class DataTransMode;
enum class TaskPlanStatus;
class QSqlQuery;
struct DeviceWorkTaskQuery;
struct DeviceWorkTask;
struct DeviceWorkTaskConditionQuery;
struct DeviceWorkTaskConditionQueryACK;
using DeviceWorkTaskList = QList<DeviceWorkTask>;
struct DataTranWorkTask;
using DataTranWorkTaskList = QList<DataTranWorkTask>;
struct TaskPlanData;
using TaskPlanDataList = QList<TaskPlanData>;
struct AutoTaskLogData;

/*
 * 两个表
 * 一个设备计划一个数传计划
 * 两个表的uuid都是  流水号_任务号
 *
 * 两个表关联
 * 先查询设备计划表得到设备计划的数据
 * 然后根据设备计划中的目标中的跟踪计划号
 * 与流水号混合关联到数传计划
 * 设备工作计划 m_uuid = m_plan_serial_number_任务号
 * 数传工作计划 m_uuid = m_plan_serial_number_任务号
 *
 * 设备工作计划 每个目标有数传计划接收号 通过数传接收计划接收号 和 m_plan_serial_number可以索引数传工作计划
 */
class TaskPlanSql
{
public:
    /* wp begin */
    /* 初始化数据库 */
    static OptionalNotValue initDB();

    /* 查询所有数据 忽略删除标记字段 */
    static Optional<DataTranWorkTaskList> queryAllDTPlan();
    static Optional<DeviceWorkTaskList> queryAllDevPlan();
    /* 查询所有数据 删除标记是已删除的不会被查出 */
    static Optional<DataTranWorkTaskList> queryAllDTPlanMark();
    static Optional<DeviceWorkTaskList> queryAllDevPlanMark();

    /* 插入数据 所有的数据插入世时 删除标记被标记为 0 */
    static Optional<TaskPlanData> insertPlan(const TaskPlanData& planWorkTask);
    static Optional<DataTranWorkTask> insertPlan(const DataTranWorkTask& pDeviceWorkTask);
    static Optional<DataTranWorkTaskList> insertPlan(const DataTranWorkTaskList& pDeviceWorkTask);
    static Optional<DeviceWorkTask> insertPlan(const DeviceWorkTask& pDeviceWorkTask);

    /* 更新计划数据 更新时会首先检查当前的被更新的计划是否处于未运行状态 如果不是则更新失败 */
    static Optional<TaskPlanData> updatePlan(const TaskPlanData& planWorkTask);

    /* 更新计划的状态,将计划以及关联的计划标记为未运行 运行 错误等等状态 */
    static OptionalNotValue updataTaskStatus(const QString& deviceTaskID, TaskPlanStatus status);

    /* 删除计划 会直接将计划从数据库中删除 */
    static OptionalNotValue deletePlan(const QString& deviceTaskID);
    static OptionalNotValue deleteDTPlan(const QString& sn);
    static OptionalNotValue deleteDevPlan(const QString& sn);
    /* 删除计划 不会将数据直接删除 只会将当前数据标记为已删除 */
    static OptionalNotValue deletePlanMark(const QString& deviceTaskID);
    static OptionalNotValue deleteDTPlanMark(const QString& sn);
    static OptionalNotValue deleteDevPlanMark(const QString& sn);

    /* 根据设备工作计划的ID查询完整的计划 忽略删除标记 */
    static Optional<TaskPlanData> queryPlanByID(const QString& deviceTaskID);
    /* 根据设备工作计划的ID查询完整的计划 不会查询标记为delete的数据 */
    static Optional<TaskPlanData> queryPlanByIDMark(const QString& deviceTaskID);

    /* 根据任务流水号查询完整的计划 这个可能同一个流水号会对应多个计划,当多个计划时认为失败 */
    static Optional<TaskPlanData> queryPlanBySN(const QString& sn);
    static Optional<TaskPlanData> queryPlanBySNMark(const QString& sn);

    /* 根据设备工作计划查询相关联的数传计划 */
    static Optional<DataTranWorkTaskList> queryPlanByDev(const DeviceWorkTask& deviceWorkTask);
    static Optional<DataTranWorkTaskList> queryPlanByDevMark(const DeviceWorkTask& deviceWorkTask);

    /* 根据流水号和跟踪号查询相关联数传计划 */
    static Optional<DataTranWorkTaskList> queryPlanBySN(const QString& sn, const QString& traceNum);
    static Optional<DataTranWorkTaskList> queryPlanBySNMark(const QString& sn, const QString& traceNum);

    /*
     * 20210509
     * wp?? 下计划的时候需要比较精确的时间
     * 但是查询计划的时候这个时间就没有什么必要了
     * 精确到天足够了
     */
    /* 查询指定范围内的设备工作计划 */
    static Optional<DeviceWorkTaskList> queryHistoryDevPlan(const QDate& startDate, const QDate& endDate);
    static Optional<DeviceWorkTaskList> queryHistoryDevPlanMark(const QDate& startDate, const QDate& endDate);

    /* 查询指定范围内的数传工作计划 */
    static Optional<DataTranWorkTaskList> queryHistoryDTPlan(const QDate& startTime, const QDate& endTime);
    static Optional<DataTranWorkTaskList> queryHistoryDTPlanMark(const QDate& startTime, const QDate& endTime);

    /* 根据条件查询历史设备工作计划 暂未使用 */
    static Optional<DeviceWorkTaskConditionQueryACK> getHistoryDevPlan(const DeviceWorkTaskConditionQuery& item);

    /* 查询运行中和未运行的设备工作计划 */
    static Optional<DeviceWorkTaskList> queryRunningOrNoStartDevPlan();
    static Optional<DeviceWorkTaskList> queryRunningOrNoStartDevPlanMark();
    /* 查询未运行的设备工作计划 */
    static Optional<DeviceWorkTaskList> queryNoStartDevPlan();
    static Optional<DeviceWorkTaskList> queryNoStartDevPlanMark();

    /* 查询运行中和未运行的数传工作计划 */
    static Optional<DataTranWorkTaskList> queryRunningOrNoStartDTPlan();
    static Optional<DataTranWorkTaskList> queryRunningOrNoStartDTPlanMark();
    /* 查询正在运行的设备工作计划 */
    static Optional<DeviceWorkTaskList> queryRunningDevPlan();
    static Optional<DeviceWorkTaskList> queryRunningDevPlanMark();
    /* 查询下一个需要运行的设备工作计划 */
    static Optional<DeviceWorkTask> queryNextNewDevPlan();
    static Optional<TaskPlanData> queryNextNewPlan();

    /* 查询未运行的数传工作计划 */
    static Optional<DataTranWorkTaskList> queryNoStartDTPlan();
    static Optional<DataTranWorkTaskList> queryNoStartDTPlanMark();

    /* 既是唯一ID也是计划流水号 */
    static Optional<QString> createPlanID();

    /* 重置计划会检索20天之前的计划然后重置未开始、正在运行的计划为错误 */
    static void resetPlan();
    static bool dtPlanIsRunning(const QString&sn);
    static bool devPlanIsRunning(const QString&sn);

    /* wp end */

    /**************************************************************************************************************/
    /**************************************************************************************************************/
    /**************************************************************************************************************/
    /**************************************************************************************************************/
    /**************************************************************************************************************/

    static OptionalNotValue initAutoTaskLogDB();
    static Optional<QList<AutoTaskLogData>> queryAutoTaskLog(const QString& uuid, quint64 startTime, quint64 endTime);
    static Optional<bool> insertAutoTaskLog(const QList<AutoTaskLogData>& autoTaskLogList);
    static Optional<bool> insertAutoTaskLog(const AutoTaskLogData& autoTaskLog);

    static bool checkTask(const QDateTime& startDateTime, const QDateTime& endDateTime, const QString& exclude = QString());
};

#endif  // TASKPLANSQL_H
