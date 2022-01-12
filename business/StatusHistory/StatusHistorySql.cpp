#include "StatusHistorySql.h"
#include "DBInterface.h"
#include "StatusPersistenceMessageSerialize.h"
#include "GlobalData.h"
#include <QDateTime>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QUuid>
#include <QMutexLocker>

const int loadNumberPerMore = 100;

StatusHistorySql::StatusHistorySql()
    : m_connectName(DBInterface::createConnectName())
{
}

QString StatusHistorySql::createTableName()
{
    return createTableName(GlobalData::currentDateTime());
}

QString StatusHistorySql::createTableName(const QDateTime& dateTime)
{
    return QString("STATUS_HISTORY_%1").arg(dateTime.toString("yyyyMMdd"));
}

QString StatusHistorySql::createUuid()
{
    return Utility::createTimeUuid();
}

OptionalNotValue StatusHistorySql::initDB(QSqlDatabase& db)
{
    /* 表名 */
    m_tableName = createTableName();
    if(m_tableName.isEmpty())
    {
        return OptionalNotValue(ErrorCode::InvalidData, "当前时间错误，设备状态历史表创建失败");
    }

    //#ifdef Q_OS_WIN
    //        QString sql = "CREATE TABLE if not exists STATUS_HISTORY (Id INT PRIMARY KEY AUTO_INCREMENT, "
    //                      "Time TIMESTAMP, ProId VARCHAR(16), DeviceId VARCHAR(16), Content longblob);";
    //#else
    //        QString sql = "CREATE TABLE if not exists STATUS_HISTORY (Id INT PRIMARY KEY AUTO_INCREMENT, "
    //                      "Time TIMESTAMP, ProId VARCHAR(16), DeviceId VARCHAR(16), Content blob)"
    //                      "PARTITION BY RANGE(Time)"
    //                      "INTERVAL(NUMTOYMINTERVAL(1, 'MONTH'))"
    //                      "(PARTITION p1 VALUES LESS THAN ('2021-1-1'));"
    //                      "CREATE UNIQUE INDEX TBL_STATUS_HISTORY_PKEY ON SYSDBA.STATUS_HISTORY "
    //                      "USING BTREE (Time, DeviceId, Id) PRIORITY 0 TABLESPACE SYSTEM "
    //                      "LOGGING INIT 64K NEXT 64K PCTINCREASE 50 FILL 70 SPLIT 50 VISIBLE;";
    //#endif

    QString sql = "CREATE TABLE if not exists %1 ("
                  "Id             varchar(128) PRIMARY KEY  ,"
                  "Time           TIMESTAMP                 ,"
                  "DeviceId       bigint                    ,"
                  "Content        blob                     );";

    auto sqlQuery = db.exec(sql.arg(m_tableName));
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return OptionalNotValue(ErrorCode::SqlExecFailed, QString("状态数据库初始化失败,错误：%1").arg(lastError.text()));
    }
    return OptionalNotValue();
}
OptionalNotValue StatusHistorySql::initDB()
{
    AutoCloseDBHelper closeHelper(m_connectName);
    {
        auto db = DBInterface::getDeviceStatusConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        return initDB(db.value());
    }
}

OptionalNotValue StatusHistorySql::insertStatus(QSqlDatabase& db, const QList<StatusRecord>& statusRecordList)
{
    if(m_tableName.isEmpty())
    {
        return OptionalNotValue(ErrorCode::InvalidData, QString("设备状态入库失败，未找到当前数据库表，丢失数据:%1条").arg(statusRecordList.size()));
    }
    if(!db.transaction())
    {
        return OptionalNotValue(ErrorCode::InvalidData, "设备状态入库，事务启动失败");
    }

    auto query = QSqlQuery(db);
    QString sql = "INSERT INTO %1 "
                  "(Id, Time, DeviceId, Content) "
                  "VALUES(?,?,?,?)";
    sql = sql.arg(m_tableName);

    query.prepare(sql);
    for(auto&item : statusRecordList)
    {
        query.bindValue(0, item.id);
        query.bindValue(1, item.dateTime);
        query.bindValue(2, item.deviceID);
        query.bindValue(3, item.json.toUtf8(), QSql::Binary);
        query.exec();
    }

    if(!db.commit())
    {
        db.rollback();
        return OptionalNotValue(ErrorCode::SqlExecFailed, "设备状态入库，事务提交失败");
    }
    return OptionalNotValue();
}

OptionalNotValue StatusHistorySql::insertStatus(const QList<StatusRecord>& statusRecordList)
{
    AutoCloseDBHelper closeHelper(m_connectName);
    {
        auto db = DBInterface::getDeviceStatusConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        return insertStatus(db.value(), statusRecordList);
    }
}

Optional<DeviceStatusLogDataList> StatusHistorySql::search(const QSet<int>& devices, const QSet<QString>& units, QDateTime startTime, QDateTime endTime, const QSet<int>& modes, int currentPage, int pageSize)
{
    using ResType = Optional<DeviceStatusLogDataList>;
    AutoCloseDBHelper closeHelper(m_connectName);
    {
        auto db = DBInterface::getDeviceStatusConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return search(db.value(), devices, units, startTime, endTime, modes, currentPage, pageSize);
    }
}
Optional<DeviceStatusLogDataList> StatusHistorySql::search(QSqlDatabase& db, const QSet<int>& devices, const QSet<QString>& units,  QDateTime startTime, QDateTime endTime, const QSet<int>& modes, int currentPage, int pageSize)
{
    /* 这里保证日期一致 表名就一致了 */
    if(startTime.date() != endTime.date())
    {
        endTime.setDate(startTime.date());
    }

    using ResType = Optional<DeviceStatusLogDataList>;
    DeviceStatusLogDataList resultList;
    DeviceStatusLogData data;

    auto query = QSqlQuery(db);

    QString sql = "select * from %1 where Id in "
                  "(select Id from "
                  "(select * from %1 where Id between '%2' and '%3' ) as tempTable1 "
                  "where (1=1) ";//注意少个括号
    sql = sql.arg(createTableName(startTime), Utility::createTimeSearchPrefix(startTime), Utility::createTimeSearchPrefix(endTime));

    if (!devices.isEmpty())
    {
        QStringList device_list_str;
        bool selAllDevice = false;

        for (auto & device: devices)
        {
            if (device <= 0)
            {
                selAllDevice = true;
                break;
            }
            device_list_str.append(QString("'%1'").arg(device));
        }

        if (!selAllDevice)
        {
            sql += QString("AND (DeviceId IN (%1)) ").arg(device_list_str.join(','));
        }
    }
    //分页条件查询
    sql += QString(" limit %1,%2 )").arg((currentPage - 1) * pageSize).arg(pageSize);
    qWarning() << "状态查询:" << sql;

    query.exec(sql);

    auto lastError = query.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    while(query.next())
    {
        auto record = query.record();
        auto time = record.field("Time").value().toDateTime();
        auto content = record.field("Content").value().toByteArray();
        auto deviceId = record.field("deviceId").value().toUInt();

        DeviceStatusData deviceStatusData;
        content >> deviceStatusData;
        //QString device_name = deviceStatusData.deviceName;

        if (!modes.isEmpty() && !modes.contains(deviceStatusData.modeId))
        {
            continue;
        }

        QString parName, value;

        for(auto iter = deviceStatusData.unitParamMap.begin(); iter != deviceStatusData.unitParamMap.end(); ++iter)
        {
            auto unitId = iter.key();
            if (!units.isEmpty() && !units.contains(unitId))
            {
                continue;
            }

            auto paramStatuss = iter->paramStatusMap;
            for (const auto& paramStatus : paramStatuss)
            {
                parName = paramStatus.parName;
                value = paramStatus.value.toString();
                data.id = loadNumberPerMore;
                data.createTime = time;
                data.deviceId = QString::number(deviceId,16);
                data.unitId = unitId;
                data.param = parName;
                if (!paramStatus.enumValueDesc.isEmpty())
                {
                    data.paramValue = paramStatus.enumValueDesc;
                }
                else
                {
                    data.paramValue = paramStatus.value.toString();
                    if (!paramStatus.unit.isEmpty())
                    {
                        data.paramValue += QString(" (%1)").arg(paramStatus.unit);
                    }
                }
                data.modeId = deviceStatusData.modeId;

                if (!data.param.contains("保留"))
                {
                    resultList.append(data);
                }
            }
        }
    }

    return ResType(resultList);
}

int StatusHistorySql::totalCount(const QSet<int>& devices, const QSet<QString>& units, QDateTime startTime, QDateTime endTime, const QSet<int>& modes)
{
    AutoCloseDBHelper closeHelper(m_connectName);
    {
        auto db = DBInterface::getDeviceStatusConnect(closeHelper());
        if (!db)
        {
            return 0;
        }
        return totalCount(db.value(), devices, units, startTime, endTime, modes);
    }
}
int StatusHistorySql::totalCount(QSqlDatabase& db, const QSet<int>& devices, const QSet<QString>& units, QDateTime startTime, QDateTime endTime, const QSet<int>& modes)
{
    /* 这里保证日期一致 表名就一致了 */
    if(startTime.date() != endTime.date())
    {
        endTime.setDate(startTime.date());
    }

    QString sql = "select count(Id) from %1 where Id in "
                  "(select Id from "
                  "(select * from %1 where Id between '%2' and '%3' ) as tempTable1 "
                  "where (1=1) ";
    sql = sql.arg(createTableName(startTime), Utility::createTimeSearchPrefix(startTime), Utility::createTimeSearchPrefix(endTime));

    if (!devices.isEmpty())
    {
        QStringList device_list_str;
        bool selAllDevice = false;

        for (auto & device: devices)
        {
            if (device <= 0)
            {
                selAllDevice = true;
                break;
            }
            device_list_str.append(QString("'%1'").arg(device));
        }

        if (!selAllDevice)
        {
            sql += QString("AND (DeviceId IN (%1)) )").arg(device_list_str.join(','));
        }
    }

    qWarning() << "状态总条数查询:" << sql;

    auto query = QSqlQuery(db);
    query.exec(sql);
    if (query.next())
    {
        return query.value(0).toInt();
    }

    return 0;
}
