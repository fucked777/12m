#include "SqlDistanceCalibration.h"
#include "DBInterface.h"
#include "DistanceCalibrationSerialize.h"
#include "GlobalData.h"
#include <QDateTime>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

OptionalNotValue SqlDistanceCalibration::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists DistanceCalibration ("
                      "id           varchar(64) PRIMARY KEY,"
                      "taskIdent    varchar(64)     ,"
                      "createTime   timestamp       ,"
                      "channel      int             ,"
                      "equipComb    bigint          ,"
                      "uplinkFreq   float           ,"
                      "downFreq     float           ,"
                      "correctValue float           ,"
                      "statisPoint  int             ,"
                      "signalRot    int             ,"
                      "distZeroMean float           ,"
                      "distZeroVar  float           ,"
                      "taskCode     varchar(64)     ,"
                      "projectCode  varchar(64)     ,"
                      "workMode     int             )";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
Optional<DistanceCalibrationItem> SqlDistanceCalibration::insertItem(const DistanceCalibrationItem& bak)
{
    using ResType = Optional<DistanceCalibrationItem>;
    auto item = bak;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "insert into DistanceCalibration("
                      "id              ,"
                      "taskIdent       ,"
                      "createTime      ,"
                      "channel         ,"
                      "equipComb       ,"
                      "uplinkFreq      ,"
                      "downFreq        ,"
                      "correctValue    ,"
                      "statisPoint     ,"
                      "signalRot       ,"
                      "distZeroMean    ,"
                      "distZeroVar     ,"
                      "taskCode        ,"
                      "projectCode     ,"
                      "workMode        )"
                      "values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
        QSqlQuery sqlQuery(db.value());
        item.createTime = GlobalData::currentDateTime();
        item.id = Utility::createTimeUuid(item.createTime);

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.id);
        sqlQuery.bindValue(1, item.taskIdent);
        sqlQuery.bindValue(2, item.createTime);
        sqlQuery.bindValue(3, item.channel);
        sqlQuery.bindValue(4, item.equipComb);
        sqlQuery.bindValue(5, item.uplinkFreq);
        sqlQuery.bindValue(6, item.downFreq);
        sqlQuery.bindValue(7, item.correctValue);
        sqlQuery.bindValue(8, item.statisPoint);
        sqlQuery.bindValue(9, item.signalRot);
        sqlQuery.bindValue(10, item.distZeroMean);
        sqlQuery.bindValue(11, item.distZeroVar);
        sqlQuery.bindValue(12, item.taskCode);
        sqlQuery.bindValue(13, item.projectCode);
        sqlQuery.bindValue(14, qint32(item.workMode));

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<DistanceCalibrationItem> SqlDistanceCalibration::deleteItem(const DistanceCalibrationItem& item)
{
    using ResType = Optional<DistanceCalibrationItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        auto sql = QString("DELETE FROM DistanceCalibration WHERE id = '%1';").arg(item.id);
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
OptionalNotValue SqlDistanceCalibration::deleteItem(const ClearDistanceCalibrationHistry& item)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }
        /* 整理的间隔时间 id是附加时间信息的uuid */
        auto currentDateTime = GlobalData::currentDateTime();
        currentDateTime = currentDateTime.addDays(-static_cast<qint64>(item.timeInterval));

        auto sql = QString("DELETE FROM DistanceCalibration WHERE id < '%1';").arg(Utility::createTimeSearchPrefix(currentDateTime));
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static QString createCondQuerySqlDistanceCalibration(const QueryDistanceCalibrationHistry& cond)
{
    QString sql;

    /* 代表所有时间 id是附加时间信息的uuid */
    if (cond.beginTime.isValid() && cond.endTime.isValid())
    {
        QString temp = " select * from DistanceCalibration where id >= '%1' And id <= '%2' ";
        sql = temp.arg(Utility::createTimeSearchPrefix(cond.beginTime), Utility::createTimeSearchPrefix(cond.endTime.addDays(1)));
    }

    if (cond.taskCode.isEmpty())
    {
        if (sql.isEmpty())
        {
            return QString("SELECT * FROM DistanceCalibration ;");
        }
        return sql;
    }

    QString sqlTemplate = "select * from "
                          "( %1 ) as temptable "
                          "where taskCode = '%2' ; ";
    return sqlTemplate.arg(sql, cond.taskCode);
}

Optional<DistanceCalibrationItemList> SqlDistanceCalibration::selectItem(const QueryDistanceCalibrationHistry& cond)
{
    using ResType = Optional<DistanceCalibrationItemList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        auto sqlQuery = db->exec(createCondQuerySqlDistanceCalibration(cond));

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DistanceCalibrationItemList itemList;
        while (sqlQuery.next())
        {
            DistanceCalibrationItem data;

            //查询数据行
            QSqlRecord recordData = sqlQuery.record();
            data.id = recordData.field("id").value().toString();
            data.taskIdent = recordData.field("taskIdent").value().toString();
            data.createTime = recordData.field("createTime").value().toDateTime();
            data.channel = recordData.field("channel").value().toInt();
            data.equipComb = recordData.field("equipComb").value().toULongLong();
            data.uplinkFreq = recordData.field("uplinkFreq").value().toDouble();
            data.downFreq = recordData.field("downFreq").value().toDouble();
            data.correctValue = recordData.field("correctValue").value().toDouble();
            data.statisPoint = recordData.field("statisPoint").value().toUInt();
            data.signalRot = recordData.field("signalRot").value().toUInt();
            data.distZeroMean = recordData.field("distZeroMean").value().toDouble();
            data.distZeroVar = recordData.field("distZeroVar").value().toDouble();
            data.taskCode = recordData.field("taskCode").value().toString();
            data.projectCode = recordData.field("projectCode").value().toString();
            data.workMode = SystemWorkMode(recordData.field("workMode").value().toInt());
            itemList << data;
        }

        return ResType(itemList);
    }
}
