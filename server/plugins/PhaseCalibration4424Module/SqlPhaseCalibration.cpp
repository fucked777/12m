#include "SqlPhaseCalibration.h"
#include "DBInterface.h"
#include "GlobalData.h"
#include "PhaseCalibrationDefine.h"
#include "PhaseCalibrationSerialize.h"
#include <QDateTime>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

static PhaseCalibrationItem queryPhaseCalibrationItem(QSqlQuery& sqlQuery)
{
    PhaseCalibrationItem data;

    //查询数据行
    QSqlRecord recordData = sqlQuery.record();

    data.id = recordData.field("id").value().toString();
    data.projectCode = recordData.field("projectCode").value().toString();
    data.taskIdent = recordData.field("taskIdent").value().toString();
    data.taskCode = recordData.field("taskCode").value().toString();
    data.createTime = recordData.field("createTime").value().toDateTime();
    data.workMode = SystemWorkMode(recordData.field("workMode").value().toInt());
    data.equipComb = recordData.field("equipComb").value().toULongLong();
    data.dotDrequency = recordData.field("dotDrequency").value().toInt();
    data.downFreq = recordData.field("downFreq").value().toDouble();
    data.azpr = recordData.field("azpr").value().toDouble();
    data.elpr = recordData.field("elpr").value().toDouble();
    data.azge = recordData.field("azge").value().toDouble();
    data.elge = recordData.field("elge").value().toDouble();
    data.temperature = recordData.field("temperature").value().toDouble();
    data.humidity = recordData.field("humidity").value().toDouble();

    auto array = recordData.field("rawParameter").value().toByteArray();
    array >> data.rawParameter;
    return data;
}

OptionalNotValue SqlPhaseCalibration::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists PhaseCalibration ("
                      "id               varchar(64) PRIMARY KEY ,"
                      "projectCode      varchar(64)             ,"
                      "taskIdent        varchar(64)             ,"
                      "taskCode         varchar(64)             ,"
                      "createTime       timestamp               ,"
                      "workMode         int                     ,"
                      "equipComb        bigint                  ,"
                      "dotDrequency     int                     ,"
                      "downFreq         float                   ,"
                      "azpr             float                   ,"
                      "elpr             float                   ,"
                      "azge             float                   ,"
                      "elge             float                   ,"
                      "temperature      float                   ,"
                      "humidity         float                   ,"
                      "rawParameter     blob                    )";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<PhaseCalibrationItem> SqlPhaseCalibration::insertItem(const PhaseCalibrationItem& bak)
{
    auto item = bak;
    using ResType = Optional<PhaseCalibrationItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "insert into PhaseCalibration("
                      "id                ,"
                      "projectCode       ,"
                      "taskIdent         ,"
                      "taskCode          ,"
                      "createTime        ,"
                      "workMode          ,"
                      "equipComb         ,"
                      "dotDrequency      ,"
                      "downFreq          ,"
                      "azpr              ,"
                      "elpr              ,"
                      "azge              ,"
                      "elge              ,"
                      "temperature       ,"
                      "humidity          ,"
                      "rawParameter      )"
                      "values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());
        item.createTime = GlobalData::currentDateTime();
        item.id = Utility::createTimeUuid(item.createTime);

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.id);
        sqlQuery.bindValue(1, item.projectCode);
        sqlQuery.bindValue(2, item.taskIdent);
        sqlQuery.bindValue(3, item.taskCode);
        sqlQuery.bindValue(4, item.createTime);
        sqlQuery.bindValue(5, item.workMode);
        sqlQuery.bindValue(6, item.equipComb);
        sqlQuery.bindValue(7, item.dotDrequency);
        sqlQuery.bindValue(8, item.downFreq);
        sqlQuery.bindValue(9, item.azpr);
        sqlQuery.bindValue(10, item.elpr);
        sqlQuery.bindValue(11, item.azge);
        sqlQuery.bindValue(12, item.elge);
        sqlQuery.bindValue(13, item.temperature);
        sqlQuery.bindValue(14, item.humidity);

        QByteArray array;
        array << item.rawParameter;
        sqlQuery.bindValue(15, array);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<PhaseCalibrationItem> SqlPhaseCalibration::deleteItem(const PhaseCalibrationItem& item)
{
    using ResType = Optional<PhaseCalibrationItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        auto sql = QString("DELETE FROM PhaseCalibration WHERE id = '%1';").arg(item.id);
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

OptionalNotValue SqlPhaseCalibration::deleteItem(const ClearPhaseCalibrationHistry& item)
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

        auto sql = QString("DELETE FROM PhaseCalibration WHERE id < '?';").arg(Utility::createTimeSearchPrefix(currentDateTime));
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

static QString createCondQuerySqlPhaseCalibration(const QueryPhaseCalibrationHistry& cond)
{
    QString sql;

    /* 代表所有时间 id是附加时间信息的uuid */
    if (cond.beginTime.isValid() && cond.endTime.isValid())
    {
        QString temp = " select * from PhaseCalibration where id >= '%1' And id <= '%2' ";
        sql = temp.arg(Utility::createTimeSearchPrefix(cond.beginTime), Utility::createTimeSearchPrefix(cond.endTime));
    }

    if (cond.taskCode.isEmpty())
    {
        if (sql.isEmpty())
        {
            return QString("SELECT * FROM PhaseCalibration ;");
        }
        return sql;
    }

    QString sqlTemplate = "select * from "
                          "( %1 ) as temptable "
                          "where taskCode = '%2' ; ";
    return sqlTemplate.arg(sql, cond.taskCode);
}

Optional<PhaseCalibrationItemList> SqlPhaseCalibration::selectItem(const QueryPhaseCalibrationHistry& cond)
{
    using ResType = Optional<PhaseCalibrationItemList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        auto sqlQuery = db->exec(createCondQuerySqlPhaseCalibration(cond));

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        PhaseCalibrationItemList itemList;
        while (sqlQuery.next())
        {
            itemList << queryPhaseCalibrationItem(sqlQuery);
        }

        return ResType(itemList);
    }
}
