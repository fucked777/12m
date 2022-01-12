#include "SqlTaskReportManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "AutoTaskMessageDefine.h"
#include "DBInterface.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskReportMessageSerialize.h"

SqlTaskReportManager::SqlTaskReportManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlTaskReportManager::~SqlTaskReportManager() {}

static TaskReportTable sqlRecordToReportItem(QSqlRecord& recordData)
{
    TaskReportTable item;
    item.uuid = recordData.field("uuid").value().toString();
    item.createTime = recordData.field("createTime").value().toDateTime().toString(DATETIME_ORIGIN_FORMAT);
    item.preStartTime = recordData.field("preStartTime").value().toString();
    item.startTime = recordData.field("startTime").value().toDateTime().toString(DATETIME_ORIGIN_FORMAT);
    item.endTime = recordData.field("endTime").value().toDateTime().toString(DATETIME_ORIGIN_FORMAT);
    item.planSerialNumber = recordData.field("planSerialNumber").value().toString();
    item.taskCode = recordData.field("taskCode").value().toString();

    return item;
}

OptionalNotValue SqlTaskReportManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS TaskReportTable("
                      "uuid VARCHAR(256) PRIMARY KEY, "
                      "createTime TIMESTAMP not null, "
                      "preStartTime TIMESTAMP not null, "
                      "startTime TIMESTAMP not null, "
                      "endTime TIMESTAMP not null, "
                      "planSerialNumber text not null, "
                      "taskCode text  not null, "
                      "allData blob"
                      ");";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "任务报表管理数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<TaskReportTableList> SqlTaskReportManager::getAllDataInfo()
{
    using ResType = Optional<TaskReportTableList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sqlTemplate = "SELECT * FROM TaskReportTable;";

        QString sql = sqlTemplate;
        QSqlQuery sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(db.errorCode(), db.msg());
        }

        TaskReportTableList dataList;
        TaskReportTable data;
        while (sqlQuery.next())
        {
            //查询数据行
            QSqlRecord record = sqlQuery.record();
            data = sqlRecordToReportItem(record);
            dataList.append(data);
        }
        return ResType(dataList);
    }
}

Optional<TaskReportTable> SqlTaskReportManager::insertData(const TaskReportTable& data)
{
    using ResType = Optional<TaskReportTable>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QSqlQuery sqlQuery(db.value());
        QString sqlTemplate = "INSERT INTO TaskReportTable (uuid, createTime, preStartTime, "
                              "startTime, endTime, planSerialNumber, taskCode, allData) "
                              "VALUES (?,?,?,?,?,?,?,?);";

        sqlQuery.prepare(sqlTemplate);
        sqlQuery.bindValue(0, data.uuid);
        sqlQuery.bindValue(1, data.createTime);
        sqlQuery.bindValue(2, data.preStartTime);
        sqlQuery.bindValue(3, data.startTime);
        sqlQuery.bindValue(4, data.endTime);
        sqlQuery.bindValue(5, data.planSerialNumber);
        sqlQuery.bindValue(6, data.taskCode);
        QByteArray json;
        json << data;
        sqlQuery.bindValue(7, json);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(db.errorCode(), db.msg());
        }

        return ResType(data);
    }
}

Optional<TaskReportTable> SqlTaskReportManager::deleteData(const TaskReportTable& data)
{
    using ResType = Optional<TaskReportTable>;
    TaskReportTable taskReport;
    return ResType(taskReport);
}

Optional<TaskReportTable> SqlTaskReportManager::updataData(const TaskReportTable& data)
{
    using ResType = Optional<TaskReportTable>;
    TaskReportTable taskReport;
    return ResType(taskReport);
}

Optional<TaskReportTableList> SqlTaskReportManager::getDataByUUID(QString uuid)
{
    using ResType = Optional<TaskReportTableList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sqlTemplate = "select * from TaskReportTable where uuid='%1';";
        QString sql = sqlTemplate.arg(uuid);
        QSqlQuery sqlQuery = db->exec(sql.toUtf8());

        bool ret = sqlQuery.lastError().isValid();
        TaskReportTableList dataList;
        if (!ret)
        {
            while (sqlQuery.next())
            {
                TaskReportTable reportTable;
                QSqlRecord record = sqlQuery.record();
                //查询数据行
                reportTable = sqlRecordToReportItem(record);

                dataList.append(reportTable);
            }
            return ResType(dataList);
        }
        return ResType(db.errorCode(), db.msg());
    }
}
