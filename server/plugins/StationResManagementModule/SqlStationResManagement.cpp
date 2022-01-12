#include "SqlStationResManagement.h"

#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

#include "CConverter.h"
#include "DBInterface.h"
#include "StationResManagementSerialize.h"

SqlStationResManagement::SqlStationResManagement(QObject* parent)
    : QObject(parent)
    , m_dbConnectName(DBInterface::createConnectName())
{
}

SqlStationResManagement::~SqlStationResManagement() {}

OptionalNotValue SqlStationResManagement::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists StationResManagement ("
                      "stationIdentifying    varchar(256) primary key ,"
                      "stationName           varchar(256) ,"
                      "ttcUACAddr            varchar(64) ,"
                      "dataTransUACAddr      varchar(64) ,"
                      "descText              varchar(512) ,"
                      "stationLongitude      float ,"
                      "stationLatitude       float ,"
                      "stationHeight         float ,"
                      "isUsable              integer );";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static bool dataExist(const StationResManagementData& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM StationResManagement where stationIdentifying='%1' ;").arg(item.stationIdentifying);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
Optional<StationResManagementData> SqlStationResManagement::insertItem(const StationResManagementData& raw)
{
    using ResType = Optional<StationResManagementData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        /* 首先判断数据是否存在 */
        if (dataExist(item, db.value()))
        {
            auto msg = QString("当前站标识对应的站已存在:%1").arg(item.stationIdentifying);
            return ResType(ErrorCode::DataExist, msg);
        }
        QString sql = "INSERT INTO StationResManagement ("
                      "stationIdentifying    ,"
                      "stationName           ,"
                      "ttcUACAddr            ,"
                      "dataTransUACAddr      ,"
                      "descText                  ,"
                      "stationLongitude      ,"
                      "stationLatitude       ,"
                      "stationHeight         ,"
                      "isUsable              )"
                      "values (?, ?, ?, ?, ?, ?, ?, ?, ?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.stationIdentifying);
        sqlQuery.bindValue(1, item.stationName);
        sqlQuery.bindValue(2, item.ttcUACAddr);
        sqlQuery.bindValue(3, item.dataTransUACAddr);
        sqlQuery.bindValue(4, item.descText);
        sqlQuery.bindValue(5, item.stationLongitude);
        sqlQuery.bindValue(6, item.stationLatitude);
        sqlQuery.bindValue(7, item.stationHeight);
        sqlQuery.bindValue(8, static_cast<quint32>(item.isUsable));
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<StationResManagementData> SqlStationResManagement::updateItem(const StationResManagementData& raw)
{
    using ResType = Optional<StationResManagementData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = "UPDATE StationResManagement "
                      "set stationName                  = ?   ,"
                      "ttcUACAddr                       = ?   ,"
                      "dataTransUACAddr                 = ?   ,"
                      "descText                         = ?   ,"
                      "stationLongitude                 = ?   ,"
                      "stationLatitude                  = ?   ,"
                      "stationHeight                    = ?   ,"
                      "isUsable                         = ?   "
                      "where stationIdentifying         = ?   ;";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, item.stationName);
        sqlQuery.bindValue(1, item.ttcUACAddr);
        sqlQuery.bindValue(2, item.dataTransUACAddr);
        sqlQuery.bindValue(3, item.descText);
        sqlQuery.bindValue(4, item.stationLongitude);
        sqlQuery.bindValue(5, item.stationLatitude);
        sqlQuery.bindValue(6, item.stationHeight);
        sqlQuery.bindValue(7, static_cast<quint32>(item.isUsable));
        sqlQuery.bindValue(8, item.stationIdentifying);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<StationResManagementData> SqlStationResManagement::deleteItem(const StationResManagementData& raw)
{
    using ResType = Optional<StationResManagementData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM StationResManagement WHERE stationIdentifying = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.stationIdentifying));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
static StationResManagementData sqlRecordToTCDataItem(QSqlRecord& record)
{
    StationResManagementData data;

    data.stationIdentifying = record.field("stationIdentifying").value().toString();
    data.stationName = record.field("stationName").value().toString();
    data.ttcUACAddr = record.field("ttcUACAddr").value().toString();
    data.dataTransUACAddr = record.field("dataTransUACAddr").value().toString();
    data.descText = record.field("descText").value().toString();
    data.stationLongitude = record.field("stationLongitude").value().toDouble();
    data.stationLatitude = record.field("stationLatitude").value().toDouble();
    data.stationHeight = record.field("stationHeight").value().toDouble();
    data.isUsable = record.field("isUsable").value().toBool();

    return data;
}
Optional<StationResManagementMap> SqlStationResManagement::selectItem()
{
    using ResType = Optional<StationResManagementMap>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM StationResManagement;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        StationResManagementMap itemMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToTCDataItem(recordData);
            itemMap[data.stationIdentifying] = data;
        }

        return ResType(itemMap);
    }
}
