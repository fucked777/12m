#include "SqlGlobalData.h"
#include "DBInterface.h"
#include "Utility.h"
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

static bool sqlGlobalDataDataExist(const QString& name, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM GlobalData where name='%1' ;").arg(name);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        return true;
    }
    return sqlQuery.next();
}

OptionalNotValue SqlGlobalData::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        return initDB(db.value());
    }
}
Optional<QByteArray> SqlGlobalData::updateItem(const QString& name, const QByteArray& value)
{
    using ResType = Optional<QByteArray>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        return updateItem(db.value(), name, value);
    }
}
OptionalNotValue SqlGlobalData::updateItemNotExist(const QString& name)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        return updateItemNotExist(db.value(), name);
    }
}
Optional<QByteArray> SqlGlobalData::selectItem(const QString& name)
{
    using ResType = Optional<QByteArray>;
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        return selectItem(db.value(), name);
    }
}
OptionalNotValue SqlGlobalData::clearItem(const QString& name)
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        return clearItem(db.value(), name);
    }
}

OptionalNotValue SqlGlobalData::initDB(QSqlDatabase& db)
{
    /* 自动化运行策略是全局数据表中的一项数据 */
    /* 这里就是表没有创建表,此项数据没有创建数据 */
    QString sql = "create table if not exists GlobalData ( "
                  "name varchar(256)    primary key      , "
                  "createTime           timestamp        , "
                  "lastTime             timestamp        , "
                  "content              blob             );";

    auto sqlQuery = db.exec(sql);
    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text());
    }

    return OptionalNotValue();
}
Optional<QByteArray> SqlGlobalData::updateItem(QSqlDatabase& db, const QString& name, const QByteArray& value)
{
    using ResType = Optional<QByteArray>;
    QSqlQuery sqlQuery(db);
    if (sqlGlobalDataDataExist(name, db))
    {
        QString sql = "UPDATE GlobalData SET "
                      "lastTime     = ?, "
                      "content      = ?  "
                      "WHERE name   = ?  ;";

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, Utility::dateTimeToStr());
        sqlQuery.bindValue(1, value);
        sqlQuery.bindValue(2, name);
    }
    else
    {
        QString sql = "insert into GlobalData("
                      "createTime         ,"
                      "lastTime           ,"
                      "content            ,"
                      "name               )"
                      "values (?,?,?,?);";

        sqlQuery.prepare(sql);
        auto timeStr = Utility::dateTimeToStr();
        sqlQuery.bindValue(0, timeStr);
        sqlQuery.bindValue(1, timeStr);
        sqlQuery.bindValue(2, value);
        sqlQuery.bindValue(3, name);
    }

    sqlQuery.exec();
    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(value);
}
OptionalNotValue SqlGlobalData::updateItemNotExist(QSqlDatabase& db, const QString& name)
{
    /* 字段存在则不管 不存在则插入 */
    QString sql = "insert into GlobalData"
                  "(createTime,lastTime,name)"
                  "select ?, ? ,? "
                  "where not exists"
                  "("
                  "    select 1 from GlobalData where name = ? "
                  ");";

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(sql);
    auto timeStr = Utility::dateTimeToStr();
    sqlQuery.bindValue(0, timeStr);
    sqlQuery.bindValue(1, timeStr);
    sqlQuery.bindValue(2, name);
    sqlQuery.bindValue(3, name);

    sqlQuery.exec();
    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
Optional<QByteArray> SqlGlobalData::selectItem(QSqlDatabase& db, const QString& name)
{
    using ResType = Optional<QByteArray>;

    auto sql = QString("SELECT * FROM GlobalData where name = '%1';").arg(name);
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    if (lastError.isValid())
    {
        return ResType(ErrorCode::SqlExecFailed, lastError.text());
    }

    if (!sqlQuery.next())
    {
        return ResType(ErrorCode::NotFound, QString("未找到索引为'%1'的数据").arg(name));
    }

    auto recordData = sqlQuery.record();
    auto array = recordData.field("content").value().toByteArray();

    return ResType(array);
}
OptionalNotValue SqlGlobalData::clearItem(QSqlDatabase& db, const QString& name)
{
    auto sql = QString("update  GlobalData set content = null where name = '%1';").arg(name);
    auto sqlQuery = db.exec(sql);

    auto lastError = sqlQuery.lastError();
    return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
}
