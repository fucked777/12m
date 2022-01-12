#include "SqlParamMacroManager.h"

#include "DBInterface.h"
#include "DMMessageSerialize.h"
#include "ParamMacroMessage.h"
#include "ParamMacroMessageSerialize.h"
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <array>

SqlParamMacroManager::SqlParamMacroManager()
    : mDbConnectName(DBInterface::createConnectName())
{
}

SqlParamMacroManager::~SqlParamMacroManager() {}
OptionalNotValue SqlParamMacroManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS ParamMacroManager("
                      "taskCode VARCHAR(100) PRIMARY KEY, "
                      "name VARCHAR(100), "
                      "workMode VARCHAR(100), "
                      "descript VARCHAR(100), "
                      "jsonData BLOB)";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "参数宏宏数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<ParamMacroData> SqlParamMacroManager::insertItem(const ParamMacroData& item)
{
    using ResType = Optional<ParamMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "INSERT INTO ParamMacroManager (taskCode, name, workMode, descript, jsonData) "
                      "VALUES (?, ?, ?, ?, ?);";
        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.taskCode);
        sqlQuery.bindValue(1, item.name);
        sqlQuery.bindValue(2, item.workMode);
        sqlQuery.bindValue(3, item.desc);

        QByteArray json;
        json << item.modeParamMap;
        sqlQuery.bindValue(4, json);

        sqlQuery.exec();

        qDebug() << QString("INSERT INTO ParamMacroManager (taskCode, name, workMode, descript, jsonData) "
                            "VALUES (%1, %2, %3, %4, %5);")
                        .arg(item.taskCode)
                        .arg(item.name)
                        .arg(item.workMode)
                        .arg(item.desc)
                        .arg("");
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ParamMacroData> SqlParamMacroManager::updateItem(const ParamMacroData& item)
{
    using ResType = Optional<ParamMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = "UPDATE ParamMacroManager SET  "
                      "name = ? , "
                      "workMode = ?, "
                      "descript = ?, "
                      "jsonData = ? "
                      "WHERE taskCode = ?;";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, item.name);
        sqlQuery.bindValue(1, item.workMode);
        sqlQuery.bindValue(2, item.desc);
        QByteArray json;
        json << item.modeParamMap;
        sqlQuery.bindValue(3, json);
        sqlQuery.bindValue(4, item.taskCode);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ParamMacroData> SqlParamMacroManager::deleteItem(const ParamMacroData& item)
{
    using ResType = Optional<ParamMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = QString("DELETE FROM ParamMacroManager WHERE taskCode = '%1';").arg(item.taskCode);
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static ParamMacroData sqlRecordToParamMacroDataItem(QSqlRecord& record)
{
    ParamMacroData item;

    item.name = record.field("name").value().toString();
    item.workMode = record.field("workMode").value().toString();
    item.taskCode = record.field("taskCode").value().toString();
    item.desc = record.field("descript").value().toString();
    auto json = record.field("jsonData").value().toByteArray();

    json >> item.modeParamMap;

    return item;
}

Optional<ParamMacroDataList> SqlParamMacroManager::selectItem()
{
    using ResType = Optional<ParamMacroDataList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM ParamMacroManager;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        ParamMacroDataList itemList;
        while (sqlQuery.next())
        {
            // 查询数据行
            auto recordData = sqlQuery.record();
            auto item = sqlRecordToParamMacroDataItem(recordData);

            itemList.append(item);
        }

        return ResType(itemList);
    }
}
