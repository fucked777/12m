#include "SqlAutomateTest.h"
#include "AutomateTestSerialize.h"
#include "DBInterface.h"
#include "GlobalData.h"
#include "Utility.h"
#include <QDateTime>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

OptionalNotValue SqlAutomateTest::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        /* 存储测试计划的表 */
        auto initResult = initTestItemDB(db.value());
        if (!initResult)
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, initResult.msg());
        }

        /* 存储测试绑定 */
        initResult = initTestBindDB(db.value());
        if (!initResult)
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, initResult.msg());
        }

        /* 存储测试结果 */
        initResult = initTestResultDB(db.value());
        if (!initResult)
        {
            return OptionalNotValue(ErrorCode::SqlExecFailed, initResult.msg());
        }

        return OptionalNotValue();
    }
}
OptionalNotValue SqlAutomateTest::initTestItemDB(QSqlDatabase& db)
{
    /* 存储测试计划的表 */
    QString sql = "create table if not exists AutomateTestItem ("
                  "id                           varchar(64) PRIMARY KEY     ,"
                  "testName                     varchar(64)                 ,"
                  "testDesc                     varchar(64)                 ,"
                  "groupName                    varchar(64)                 ,"
                  "testTypeID                   varchar(64)                 ,"
                  "testTypeName                 varchar(64)                 ,"
                  "projectCode                  varchar(64)                 ,"
                  "taskIdent                    varchar(64)                 ,"
                  "taskCode                     varchar(64)                 ,"
                  "createTime                   timestamp                   ,"
                  "workMode                     int                         ,"
                  "equipComb                    bigint                      ,"
                  "dotDrequency                 int                         ,"
                  "testParameterMap             blob                        ,"
                  "automateTestParameterBind    blob                        );";

    auto sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
OptionalNotValue SqlAutomateTest::initTestBindDB(QSqlDatabase& db)
{
    QString sql = "create table if not exists AutomateTestParameterBind ("
                  "id                   varchar(64) PRIMARY KEY         ,"
                  "testTypeID           varchar(256)                    ,"
                  "createTime           timestamp                       ,"
                  "param                blob                            );";

    /* 这里的id不是uuid,是根据当前的参数绑定的参数综合出来的一个唯一数据 */
    auto sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
OptionalNotValue SqlAutomateTest::initTestResultDB(QSqlDatabase& db)
{
    QString sql = "create table if not exists AutomateTestResult ("
                  "id                   varchar(64) PRIMARY KEY         ,"
                  "testTypeID           varchar(256)                    ,"
                  "testTypeName         varchar(256)                    ,"
                  "testPlanName         varchar(256)                    ,"
                  "testTime             timestamp                       ,"
                  "testParam            blob                            ,"
                  "testResultList       blob                            );";

    /* 这里的id不是uuid,是根据当前的参数绑定的参数综合出来的一个唯一数据 */
    auto sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
Optional<AutomateTestPlanItem> SqlAutomateTest::insertTestItem(const AutomateTestPlanItem& item)
{
    using ResType = Optional<AutomateTestPlanItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "insert into AutomateTestItem("
                      "id                        ,"
                      "testName                  ,"
                      "testDesc                  ,"
                      "testTypeID                ,"
                      "testTypeName              ,"
                      "projectCode               ,"
                      "taskIdent                 ,"
                      "taskCode                  ,"
                      "createTime                ,"
                      "workMode                  ,"
                      "equipComb                 ,"
                      "dotDrequency              ,"
                      "groupName                 ,"
                      "testParameterMap          ,"
                      "automateTestParameterBind )"
                      "values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.id);
        sqlQuery.bindValue(1, item.testName);
        sqlQuery.bindValue(2, item.testDesc);
        sqlQuery.bindValue(3, item.testTypeID);
        sqlQuery.bindValue(4, item.testTypeName);
        sqlQuery.bindValue(5, item.projectCode);
        sqlQuery.bindValue(6, item.taskIdent);
        sqlQuery.bindValue(7, item.taskCode);
        sqlQuery.bindValue(8, Utility::dateTimeToStr(item.createTime));
        sqlQuery.bindValue(9, item.workMode);
        sqlQuery.bindValue(10, item.equipComb);
        sqlQuery.bindValue(11, item.dotDrequency);
        sqlQuery.bindValue(12, item.groupName);

        QByteArray byteArray;
        byteArray << item.testParameterMap;
        sqlQuery.bindValue(13, byteArray);

        byteArray.clear();
        byteArray << item.automateTestParameterBind;
        sqlQuery.bindValue(14, byteArray);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static QString getParamPlaceholder(int count)
{
    QString res;
    for (int i = 0; i < count; ++i)
    {
        res += "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) ,";
    }
    if (!res.isEmpty())
    {
        res.chop(1);
    }
    return res;
}

Optional<AutomateTestPlanItemList> SqlAutomateTest::insertTestItem(const AutomateTestPlanItemList& items)
{
    using ResType = Optional<AutomateTestPlanItemList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "insert into AutomateTestItem("
                      "id                        ,"
                      "testName                  ,"
                      "testDesc                  ,"
                      "testTypeID                ,"
                      "testTypeName              ,"
                      "projectCode               ,"
                      "taskIdent                 ,"
                      "taskCode                  ,"
                      "createTime                ,"
                      "workMode                  ,"
                      "equipComb                 ,"
                      "dotDrequency              ,"
                      "groupName                 ,"
                      "testParameterMap          ,"
                      "automateTestParameterBind )"
                      "VALUES %1;";
        sql = sql.arg(getParamPlaceholder(items.size()));

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        int i = 0;
        for (auto& item : items)
        {
            auto tempNum = i * 15;
            sqlQuery.bindValue(0 + tempNum, item.id);
            sqlQuery.bindValue(1 + tempNum, item.testName);
            sqlQuery.bindValue(2 + tempNum, item.testDesc);
            sqlQuery.bindValue(3 + tempNum, item.testTypeID);
            sqlQuery.bindValue(4 + tempNum, item.testTypeName);
            sqlQuery.bindValue(5 + tempNum, item.projectCode);
            sqlQuery.bindValue(6 + tempNum, item.taskIdent);
            sqlQuery.bindValue(7 + tempNum, item.taskCode);
            sqlQuery.bindValue(8 + tempNum, Utility::dateTimeToStr(item.createTime));
            sqlQuery.bindValue(9 + tempNum, item.workMode);
            sqlQuery.bindValue(10 + tempNum, item.equipComb);
            sqlQuery.bindValue(11 + tempNum, item.dotDrequency);
            sqlQuery.bindValue(12 + tempNum, item.groupName);

            QByteArray byteArray;
            byteArray << item.testParameterMap;
            sqlQuery.bindValue(13 + tempNum, byteArray);

            byteArray.clear();
            byteArray << item.automateTestParameterBind;
            sqlQuery.bindValue(14 + tempNum, byteArray);

            ++i;
        }

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(items);
    }
}

Optional<AutomateTestPlanItem> SqlAutomateTest::deleteTestItem(const AutomateTestPlanItem& item)
{
    using ResType = Optional<AutomateTestPlanItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM AutomateTestItem WHERE id = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.id));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<AutomateTestPlanItem> SqlAutomateTest::deleteHistoryTestItem(const AutomateTestPlanItem& item)
{
    using ResType = Optional<AutomateTestPlanItem>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = QString("DELETE FROM automatetestresult WHERE id < '%1';")
                          .arg(Utility::createTimeSearchPrefix(GlobalData::currentDateTime().addDays(-90)));
        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<AutomateTestPlanItemList> SqlAutomateTest::queryTestItem()
{
    using ResType = Optional<AutomateTestPlanItemList>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM AutomateTestItem;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        AutomateTestPlanItemList itemList;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();

            AutomateTestPlanItem data;
            data.id = recordData.field("id").value().toString();
            data.testName = recordData.field("testName").value().toString();
            data.testDesc = recordData.field("testDesc").value().toString();
            data.testTypeID = recordData.field("testTypeID").value().toString();
            data.testTypeName = recordData.field("testTypeName").value().toString();
            data.projectCode = recordData.field("projectCode").value().toString();
            data.taskIdent = recordData.field("taskIdent").value().toString();
            data.taskCode = recordData.field("taskCode").value().toString();
            data.createTime = recordData.field("createTime").value().toDateTime();
            data.workMode = SystemWorkMode(recordData.field("workMode").value().toInt());
            data.equipComb = recordData.field("equipComb").value().toULongLong();
            data.dotDrequency = recordData.field("dotDrequency").value().toInt();
            data.groupName = recordData.field("groupName").value().toString();

            auto byteArray = recordData.field("testParameterMap").value().toByteArray();
            byteArray >> data.testParameterMap;

            byteArray = recordData.field("automateTestParameterBind").value().toByteArray();
            byteArray >> data.automateTestParameterBind;

            itemList << data;
        }

        return ResType(itemList);
    }
}
static bool dataExist(const AutomateTestParameterBind& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM AutomateTestParameterBind where id='%1' ;").arg(item.id);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
Optional<AutomateTestParameterBind> SqlAutomateTest::updateTestBind(const AutomateTestParameterBind& item)
{
    using ResType = Optional<AutomateTestParameterBind>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QSqlQuery sqlQuery(db.value());
        if (dataExist(item, db.value()))
        {
            /* 存在,则更新 */
            QString sql = "update AutomateTestParameterBind set"
                          "       testTypeID    = ?   ,"
                          "       param         = ?   "
                          "WHERE  id            = ?   ;";
            sqlQuery.prepare(sql);
            sqlQuery.bindValue(0, item.testTypeID);
            QByteArray content;
            content << item;
            sqlQuery.bindValue(1, content);
            sqlQuery.bindValue(2, item.id);
        }
        else
        {
            QString sql = "insert into AutomateTestParameterBind("
                          "id               ,"
                          "testTypeID       ,"
                          "createTime       ,"
                          "param            )"
                          "values (?,?,?,?);";

            sqlQuery.prepare(sql);
            sqlQuery.bindValue(0, item.id);
            sqlQuery.bindValue(1, item.testTypeID);
            sqlQuery.bindValue(2, Utility::dateTimeToStr(item.createTime));

            QByteArray content;
            content << item;
            sqlQuery.bindValue(3, content);
        }

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<AutomateTestParameterBind> SqlAutomateTest::deleteTestBind(const AutomateTestParameterBind& item)
{
    using ResType = Optional<AutomateTestParameterBind>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM AutomateTestParameterBind WHERE id = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.id));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<AutomateTestParameterBindMap> SqlAutomateTest::queryTestBind()
{
    using ResType = Optional<AutomateTestParameterBindMap>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM AutomateTestParameterBind;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        AutomateTestParameterBindMap groupMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();

            AutomateTestParameterBind automateTestParameterBind;
            auto tempData = recordData.field("param").value().toByteArray();
            tempData >> automateTestParameterBind;

            auto& group = groupMap[automateTestParameterBind.testTypeID];

            group.testTypeID = automateTestParameterBind.testTypeID;
            group.bindDataList << automateTestParameterBind;
        }

        return ResType(groupMap);
    }
}
Optional<AutomateTestResult> SqlAutomateTest::insertTestResultItem(const AutomateTestResult& item)
{
    using ResType = Optional<AutomateTestResult>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "insert into AutomateTestResult("
                      "id                        ,"
                      "testTypeID                ,"
                      "testTypeName              ,"
                      "testPlanName              ,"
                      "testTime                  ,"
                      "testParam                 ,"
                      "testResultList            )"
                      "values (?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.id);
        sqlQuery.bindValue(1, item.testTypeID);
        sqlQuery.bindValue(2, item.testTypeName);
        sqlQuery.bindValue(3, item.testPlanName);
        sqlQuery.bindValue(4, item.testTime);

        QByteArray array;
        array << item.testParam;
        sqlQuery.bindValue(5, array);

        array.clear();
        array << item.testResultList;
        sqlQuery.bindValue(6, array);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<AutomateTestResult> SqlAutomateTest::deleteTestResultItem(const AutomateTestResult& item)
{
    using ResType = Optional<AutomateTestResult>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM AutomateTestResult WHERE id = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.id));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static QString createCondQuerySqlDistanceCalibration(const AutomateTestResultQuery& cond)
{
    QString sql;

    /* 代表所有时间 id是附加时间信息的uuid */
    if (cond.beginTime.isValid() && cond.endTime.isValid())
    {
        QString temp = " select * from AutomateTestResult where id >= '%1' And id <= '%2' ";
        sql = temp.arg(Utility::createTimeSearchPrefix(cond.beginTime), Utility::createTimeSearchPrefix(cond.endTime.addDays(1)));
    }

    if (cond.testTypeID.isEmpty())
    {
        if (sql.isEmpty())
        {
            return QString("SELECT * FROM AutomateTestResult ;");
        }
        return sql;
    }

    QString sqlTemplate = "select * from "
                          "( %1 ) as temptable "
                          "where testTypeID = '%2' ; ";
    return sqlTemplate.arg(sql, cond.testTypeID);
}

Optional<AutomateTestResultList> SqlAutomateTest::queryTestResultItem(const AutomateTestResultQuery& cond)
{
    using ResType = Optional<AutomateTestResultList>;
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

        AutomateTestResultList itemList;
        while (sqlQuery.next())
        {
            AutomateTestResult data;

            //查询数据行
            QSqlRecord recordData = sqlQuery.record();
            data.id = recordData.field("id").value().toString();
            data.testTypeID = recordData.field("testTypeID").value().toString();
            data.testTypeName = recordData.field("testTypeName").value().toString();
            data.testPlanName = recordData.field("testPlanName").value().toString();
            data.testTime = recordData.field("testTime").value().toDateTime();

            auto array = recordData.field("testParam").value().toByteArray();
            array >> data.testParam;

            array = recordData.field("testResultList").value().toByteArray();
            array >> data.testResultList;

            itemList << data;
        }

        return ResType(itemList);
    }
}
