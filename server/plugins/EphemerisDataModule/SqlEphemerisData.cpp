#include "SqlEphemerisData.h"
#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

#include "CConverter.h"
#include "DBInterface.h"
#include "DataTransmissionCenterSerialize.h"
#include "EphemerisDataSerialize.h"

SqlEphemerisData::SqlEphemerisData(QObject* parent)
    : QObject(parent)
    , m_dbConnectName(DBInterface::createConnectName())
{
}

SqlEphemerisData::~SqlEphemerisData() {}

OptionalNotValue SqlEphemerisData::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists EphemerisData ("
                      "satellitCode               varchar(64) primary key ,"
                      "dateOfEpoch                date                    ,"
                      "timeOfEpoch                time                    ,"
                      "semimajorAxis              float                   ,"
                      "eccentricity               float                   ,"
                      "dipAngle                   float                   ,"
                      "rightAscension             float                   ,"
                      "argumentOfPerigee          float                   ,"
                      "meanAnomaly                float                  );";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static bool dataExist(const EphemerisDataData& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM EphemerisData where satellitCode='%1' ;").arg(item.satellitCode);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
Optional<EphemerisDataData> SqlEphemerisData::insertItem(const EphemerisDataData& raw)
{
    using ResType = Optional<EphemerisDataData>;
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
            auto msg = QString("当前名称对应的星历数据已存在:%1").arg(item.satellitCode);
            return ResType(ErrorCode::DataExist, msg);
        }
        QString sql = "insert into EphemerisData("
                      "satellitCode               ,"
                      "dateOfEpoch                ,"
                      "timeOfEpoch                ,"
                      "semimajorAxis              ,"
                      "eccentricity               ,"
                      "dipAngle                   ,"
                      "rightAscension             ,"
                      "argumentOfPerigee          ,"
                      "meanAnomaly                )"
                      "values (?,?,?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.satellitCode);
        sqlQuery.bindValue(1, item.dateOfEpoch.toString("yyyy-MM-dd"));
        sqlQuery.bindValue(2, item.timeOfEpoch.toString("HH:mm:ss"));
        sqlQuery.bindValue(3, item.semimajorAxis);
        sqlQuery.bindValue(4, item.eccentricity);
        sqlQuery.bindValue(5, item.dipAngle);
        sqlQuery.bindValue(6, item.rightAscension);
        sqlQuery.bindValue(7, item.argumentOfPerigee);
        sqlQuery.bindValue(8, item.meanAnomaly);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<EphemerisDataData> SqlEphemerisData::updateItem(const EphemerisDataData& raw)
{
    using ResType = Optional<EphemerisDataData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = "update EphemerisData "
                      "set dateOfEpoch              = ?   ,"
                      "timeOfEpoch                  = ?   ,"
                      "semimajorAxis                = ?   ,"
                      "eccentricity                 = ?   ,"
                      "dipAngle                     = ?   ,"
                      "rightAscension               = ?   ,"
                      "argumentOfPerigee            = ?   ,"
                      "meanAnomaly                  = ?   "
                      "where satellitCode           = ?   ;";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.dateOfEpoch.toString("yyyy-MM-dd"));
        sqlQuery.bindValue(1, item.timeOfEpoch.toString("HH:mm:ss"));
        sqlQuery.bindValue(2, item.semimajorAxis);
        sqlQuery.bindValue(3, item.eccentricity);
        sqlQuery.bindValue(4, item.dipAngle);
        sqlQuery.bindValue(5, item.rightAscension);
        sqlQuery.bindValue(6, item.argumentOfPerigee);
        sqlQuery.bindValue(7, item.meanAnomaly);
        sqlQuery.bindValue(8, item.satellitCode);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<EphemerisDataData> SqlEphemerisData::deleteItem(const EphemerisDataData& raw)
{
    using ResType = Optional<EphemerisDataData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM EphemerisData WHERE satellitCode = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.satellitCode));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
static EphemerisDataData sqlRecordToEDDataItem(QSqlRecord& record)
{
    EphemerisDataData data;
    data.satellitCode = record.field("satellitCode").value().toString();
    data.dateOfEpoch = record.field("dateOfEpoch").value().toDate();
    data.timeOfEpoch = record.field("timeOfEpoch").value().toTime();
    data.semimajorAxis = record.field("semimajorAxis").value().toDouble();
    data.eccentricity = record.field("eccentricity").value().toDouble();
    data.dipAngle = record.field("dipAngle").value().toDouble();
    data.rightAscension = record.field("rightAscension").value().toDouble();
    data.argumentOfPerigee = record.field("argumentOfPerigee").value().toDouble();
    data.meanAnomaly = record.field("meanAnomaly").value().toDouble();

    return data;
}
Optional<EphemerisDataMap> SqlEphemerisData::selectItem()
{
    using ResType = Optional<EphemerisDataMap>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM EphemerisData;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        EphemerisDataMap itemMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToEDDataItem(recordData);
            itemMap[data.satellitCode] = data;
        }

        return ResType(itemMap);
    }
}
