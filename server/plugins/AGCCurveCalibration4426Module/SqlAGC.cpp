#include "SqlAGC.h"
#include "AGCDefine.h"
#include "AGCSerialize.h"
#include "DBInterface.h"
#include "GlobalData.h"
#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

SqlAGC::SqlAGC()
    : m_dbConnectName(DBInterface::createConnectName())
{
}

SqlAGC::~SqlAGC() {}

OptionalNotValue SqlAGC::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        QString sql = "create table if not exists AGCCurveCalibration ("
                      "itemGUID             varchar(64) PRIMARY KEY     ,"
                      "createTime           timestamp                   ,"
                      "taskCode             varchar(64)                 ,"
                      "projectCode          varchar(64)                 ,"
                      "taskID               varchar(64)                 ,"
                      "workMode             int                         ,"
                      "dotDrequencyNum      int                         ,"
                      "equipComb            bigint                      ,"
                      "result               blob                        );";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
        }

        return OptionalNotValue();
    }
}
Optional<AGCCalibrationItem> SqlAGC::insertAGCItem(const AGCCalibrationItem& bak, QSqlDatabase& db)
{
    auto item = bak;
    item.createTime = GlobalData::currentDateTime();
    item.itemGUID = QString("%1-%2").arg(item.createTime.toString("yyyyMMdd"), QUuid::createUuid().toString());

    using ResType = Optional<AGCCalibrationItem>;
    if (item.taskCode.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前数据无效,无任务代号相关信息");
    }
    if (item.result.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前插入的数据为空");
    }

    QSqlQuery sqlQuery(db);
    QString sql = "INSERT INTO AGCCurveCalibration ("
                  "itemGUID        ,"
                  "createTime      ,"
                  "taskCode        ,"
                  "projectCode     ,"
                  "taskID          ,"
                  "workMode        ,"
                  "dotDrequencyNum ,"
                  "equipComb       ,"
                  "result          )"
                  "VALUES (?,?,?,?,?,?,?,?,?);";

    /* ID的格式是   yyyyMMdd-uuid  */
    sqlQuery.prepare(sql);
    sqlQuery.bindValue(0, item.itemGUID);
    sqlQuery.bindValue(1, Utility::dateTimeToStr(item.createTime));
    sqlQuery.bindValue(2, item.taskCode);
    sqlQuery.bindValue(3, item.projectCode);
    sqlQuery.bindValue(4, item.taskID);
    sqlQuery.bindValue(5, int(item.workMode));
    sqlQuery.bindValue(6, item.dotDrequencyNum);
    sqlQuery.bindValue(7, item.equipComb);

    QByteArray array;
    array << item.result;
    sqlQuery.bindValue(8, array);

    sqlQuery.exec();

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    return ResType(item);
}
Optional<AGCCalibrationItem> SqlAGC::insertAGCItem(const AGCCalibrationItem& item)
{
    using ResType = Optional<AGCCalibrationItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return insertAGCItem(item, db.value());
    }
}
Optional<AGCCalibrationItem> SqlAGC::updateAGCItem(const AGCCalibrationItem& item, QSqlDatabase& db)
{
    using ResType = Optional<AGCCalibrationItem>;
    if (item.itemGUID.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前AGC数据不存在,无法更新");
    }

    if (item.result.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前更新的数据为空");
    }

    using ResType = Optional<AGCCalibrationItem>;

    QSqlQuery sqlQuery(db);
    QString sql = "update AGCCurveCalibration set "
                  "result = ? "
                  "WHERE itemGUID = ? ";

    sqlQuery.prepare(sql);

    QByteArray array;
    array << item.result;
    sqlQuery.bindValue(0, array);
    sqlQuery.bindValue(1, item.itemGUID);

    sqlQuery.exec();

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    return ResType(item);
}
Optional<AGCCalibrationItem> SqlAGC::updateAGCItem(const AGCCalibrationItem& item)
{
    using ResType = Optional<AGCCalibrationItem>;
    if (item.itemGUID.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "当前AGC数据不存在,无法更新");
    }

    using ResType = Optional<AGCCalibrationItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        return updateAGCItem(item, db.value());
    }
}

bool SqlAGC::dataExist(const AGCCalibrationItem& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM AGCCurveCalibration where id='%1' ;").arg(item.itemGUID);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        return true;
    }
    return sqlQuery.next();
}

Optional<AGCCalibrationItem> SqlAGC::saveAGCItem(const AGCCalibrationItem& item)
{
    using ResType = Optional<AGCCalibrationItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 为空就是新插入的 */
        if (item.itemGUID.isEmpty())
        {
            return insertAGCItem(item, db.value());
        }
        /* 存在则插入否则就更新 */
        if (dataExist(item, db.value()))
        {
            return updateAGCItem(item, db.value());
        }
        return insertAGCItem(item, db.value());
    }
}

Optional<AGCCalibrationItem> SqlAGC::deleteAGCItem(const AGCCalibrationItem& item)
{
    using ResType = Optional<AGCCalibrationItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM AGCCurveCalibration WHERE itemGUID = '%1';");
        sql = sql.arg(item.itemGUID);

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        return ResType(item);
    }
}
OptionalNotValue SqlAGC::deleteAGCItem(const ClearAGCCalibrationHistry& item)
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        /* 整理的间隔时间 */
        auto curDateTime = GlobalData::currentDateTime();
        curDateTime = curDateTime.addDays(-static_cast<qint64>(item.timeInterval));

        /* ID的格式是   yyyyMMdd-uuid  */
        QString sql("DELETE FROM AGCCurveCalibration WHERE itemGUID < '%1';");
        sql = sql.arg(curDateTime.toString("yyyyMMdd"));

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
        }

        return OptionalNotValue();
    }
}

static QString createAGCResultQuery(const AGCResultQuery& cond)
{
    QString sql;
    /* 代表所有时间 id是附加时间信息的uuid */
    if (cond.beginTime.isValid() && cond.endTime.isValid())
    {
        QString temp = "select * from AGCCurveCalibration where itemGUID >= '%1' and itemGUID <= '%2'";
        sql = temp.arg(Utility::createTimeSearchPrefix(cond.beginTime), Utility::createTimeSearchPrefix(cond.endTime.addDays(1)));
    }

    if (cond.taskCode.isEmpty())
    {
        if (sql.isEmpty())
        {
            return QString("SELECT * FROM AGCCurveCalibration ;");
        }
        return sql;
    }

    QString sqlTemplate = "select * from "
                          "( %1 ) as temptable "
                          "where taskCode = '%2' ; ";
    return sqlTemplate.arg(sql, cond.taskCode);
}

Optional<AGCCalibrationItemList> SqlAGC::selectAGCItem(const AGCResultQuery& cond)
{
    using ResType = Optional<AGCCalibrationItemList>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        auto sqlQuery = db->exec(createAGCResultQuery(cond));
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        AGCCalibrationItemList result;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            AGCCalibrationItem data;
            data.itemGUID = recordData.field("itemGUID").value().toString();
            data.createTime = recordData.field("createTime").value().toDateTime();
            data.taskCode = recordData.field("taskCode").value().toString();
            data.projectCode = recordData.field("projectCode").value().toString();
            data.taskID = recordData.field("taskID").value().toString();
            data.workMode = SystemWorkMode(recordData.field("workMode").value().toUInt());
            data.dotDrequencyNum = recordData.field("dotDrequencyNum").value().toUInt();
            data.equipComb = recordData.field("equipComb").value().toULongLong();

            auto array = recordData.field("result").value().toByteArray();
            array >> data.result;

            result << data;
        }

        return ResType(result);
    }
}
