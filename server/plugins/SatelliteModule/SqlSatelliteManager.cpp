#include "SqlSatelliteManager.h"
#include "DBInterface.h"
#include "DMMessageSerialize.h"
#include "SatelliteManagementDefine.h"
#include "SatelliteManagementSerialize.h"

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <array>

SqlSatelliteManager::SqlSatelliteManager()
    : m_dbConnectName(DBInterface::createConnectName())
{
}

SqlSatelliteManager::~SqlSatelliteManager() {}
OptionalNotValue SqlSatelliteManager::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists SatelliteManager(SatelliteCode VARCHAR(100) primary key, SatelliteName text NOT NULL,"
                      " SatelliteIdentification VARCHAR(8) NOT NULL"
                      " ,StationID text ,TaskCenterID text ,DigitalCenterID text, RailType int, TrackingMode int, SPhasing int, KaPhasing int, "
                      " WorkMode text, Descript text,JsonData BLOB);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "配置宏数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

OptionalNotValue SqlSatelliteManager::insertItem(SatelliteManagementData& item)
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "INSERT INTO SatelliteManager (SatelliteCode, SatelliteName , SatelliteIdentification, StationID, TaskCenterID , "
                      "DigitalCenterID ,RailType,TrackingMode,SPhasing,KaPhasing,WorkMode, Descript,JsonData)"
                      "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.m_satelliteCode);
        sqlQuery.bindValue(1, item.m_satelliteName);
        sqlQuery.bindValue(2, item.m_satelliteIdentification);
        sqlQuery.bindValue(3, item.m_stationID);
        sqlQuery.bindValue(4, item.m_taskCenterID);
        sqlQuery.bindValue(5, item.m_digitalCenterID);
        sqlQuery.bindValue(6, int(item.m_railType));
        sqlQuery.bindValue(7, int(item.m_trackingMode));
        sqlQuery.bindValue(8, item.m_sPhasing);
        sqlQuery.bindValue(9, item.m_kaPhasing);
        sqlQuery.bindValue(10, item.m_workMode);
        sqlQuery.bindValue(11, item.m_desc);
        QByteArray json;
        json << item.m_workModeParamMap;
        sqlQuery.bindValue(12, json);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

OptionalNotValue SqlSatelliteManager::updateItem(SatelliteManagementData& item)
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql =
            "UPDATE SatelliteManager SET  SatelliteName= ? , SatelliteIdentification= ?, StationID= ?, TaskCenterID = ?, "
            "DigitalCenterID = ?,RailType= ?,TrackingMode= ?,SPhasing= ?,KaPhasing= ?,WorkMode= ?, Descript= ?,JsonData= ? WHERE SatelliteCode = ?;";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, item.m_satelliteName);
        sqlQuery.bindValue(1, item.m_satelliteIdentification);
        sqlQuery.bindValue(2, item.m_stationID);
        sqlQuery.bindValue(3, item.m_taskCenterID);
        sqlQuery.bindValue(4, item.m_digitalCenterID);
        sqlQuery.bindValue(5, int(item.m_railType));
        sqlQuery.bindValue(6, int(item.m_trackingMode));
        sqlQuery.bindValue(7, item.m_sPhasing);
        sqlQuery.bindValue(8, item.m_kaPhasing);
        sqlQuery.bindValue(9, item.m_workMode);
        sqlQuery.bindValue(10, item.m_desc);
        QByteArray json;
        json << item.m_workModeParamMap;
        sqlQuery.bindValue(11, json);
        sqlQuery.bindValue(12, item.m_satelliteCode);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

OptionalNotValue SqlSatelliteManager::deleteItem(const SatelliteManagementData& item)
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM SatelliteManager WHERE SatelliteCode = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.m_satelliteCode));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

static SatelliteManagementData sqlRecordToSatelliteDataItem(QSqlRecord& record)
{
    SatelliteManagementData data;

    QString m_workMode;  //当前卫星的工作模式  默认采用1+2+3+4形式代替，具体的值由配置决定
    QMap<int, SatelliteManagementParamItem> m_workModeParamMap;

    data.m_satelliteName = record.field("SatelliteName").value().toString();
    data.m_satelliteCode = record.field("SatelliteCode").value().toString();
    data.m_satelliteIdentification = record.field("SatelliteIdentification").value().toString();
    data.m_stationID = record.field("StationID").value().toString();
    data.m_taskCenterID = record.field("TaskCenterID").value().toString();
    data.m_digitalCenterID = record.field("DigitalCenterID").value().toString();
    data.m_railType = SatelliteRailType(record.field("RailType").value().toInt());
    data.m_trackingMode = SatelliteTrackingMode(record.field("TrackingMode").value().toInt());
    data.m_sPhasing = record.field("SPhasing").value().toBool();
    data.m_kaPhasing = record.field("KaPhasing").value().toBool();
    data.m_workMode = record.field("WorkMode").value().toString();
    data.m_desc = record.field("Descript").value().toString();
    auto m_json = record.field("JsonData").value().toByteArray();
    m_json >> data.m_workModeParamMap;

    return data;
}

Optional<QList<SatelliteManagementData>> SqlSatelliteManager::selectItem()
{
    using ResType = Optional<QList<SatelliteManagementData>>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM SatelliteManager;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        QList<SatelliteManagementData> itemMap;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToSatelliteDataItem(recordData);

            itemMap.append(data);
        }

        return ResType(itemMap);
    }
}
