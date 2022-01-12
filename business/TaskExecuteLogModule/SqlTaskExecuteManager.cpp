#include "SqlTaskExecuteManager.h"
#include "AutoRunTaskLogMessageSerialize.h"
#include "DBInterface.h"
#include "TaskPlanMessageSerialize.h"
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

class TaskExecuteSqlPrivate
{
public:
    static DeviceWorkTask sqlQueryToDevData(QSqlQuery& sqlQuery);
};

DeviceWorkTask TaskExecuteSqlPrivate::sqlQueryToDevData(QSqlQuery& sqlQuery)
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

SqlTaskExecuteManager::SqlTaskExecuteManager() {}

Optional<DeviceWorkTaskConditionQueryACK> SqlTaskExecuteManager::getHistoryDevPlan(const DeviceWorkTaskConditionQuery& item)
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

            auto lastError = sqlQuery.lastError();
            if (lastError.isValid())
            {
                return ResType(db.errorCode(), db.msg());
            }

            QList<DeviceWorkTask> items;

            while (sqlQuery.next())
            {
                //查询数据行
                items << TaskExecuteSqlPrivate::sqlQueryToDevData(sqlQuery);
            }

            ack.totalCount = totalCount;
            ack.currentPage = item.currentPage;
            ack.deviceWorkTasks = items;
            return ResType(ack);
        }
    }
}

Optional<QList<AutoTaskLogData>> SqlTaskExecuteManager::queryAutoTaskLog(const QString& uuid, quint64 startTime, quint64 endTime)
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
