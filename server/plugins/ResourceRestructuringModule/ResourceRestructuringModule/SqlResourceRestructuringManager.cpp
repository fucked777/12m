#include "SqlResourceRestructuringManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "DBInterface.h"
#include "ResourceRestructuringMessageSerialize.h"

SqlResourceRestructuringManager::SqlResourceRestructuringManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlResourceRestructuringManager::~SqlResourceRestructuringManager() {}
OptionalNotValue SqlResourceRestructuringManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS ResourceRestructuringManager("
                      "ResourceRestructuringID VARCHAR(100) PRIMARY KEY, "
                      "name VARCHAR(50), "
                      "workMode INT, "
                      "isMaster INT, "
                      "descs VARCHAR(100), "
                      "jsonData BLOB);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<ResourceRestructuringData> SqlResourceRestructuringManager::insertItem(const ResourceRestructuringData& item)
{
    using ResType = Optional<ResourceRestructuringData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "INSERT INTO ResourceRestructuringManager (ResourceRestructuringID, name, workMode, isMaster, descs, jsonData)"
                      "VALUES (?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.resourceRestructuringID);
        sqlQuery.bindValue(1, item.name);
        sqlQuery.bindValue(2, (int)item.workMode);
        sqlQuery.bindValue(3, static_cast<int>(item.isMaster));
        sqlQuery.bindValue(4, item.desc);
        QByteArray json;
        json << item.deviceMap;
        sqlQuery.bindValue(5, json);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ResourceRestructuringData> SqlResourceRestructuringManager::updateItem(const ResourceRestructuringData& item)
{
    using ResType = Optional<ResourceRestructuringData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = QString("UPDATE ResourceRestructuringManager SET "
                              "name = ?, "
                              "workMode = ?, "
                              "isMaster = ?, "
                              "descs = ? , "
                              "jsonData = ? "
                              "WHERE ResourceRestructuringID = ?;");

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, item.name);
        sqlQuery.bindValue(1, (int)item.workMode);
        sqlQuery.bindValue(2, static_cast<int>(item.isMaster));
        sqlQuery.bindValue(3, item.desc);
        QByteArray json;
        json << item.deviceMap;
        sqlQuery.bindValue(4, json);
        sqlQuery.bindValue(5, item.resourceRestructuringID);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ResourceRestructuringData> SqlResourceRestructuringManager::deleteItem(const ResourceRestructuringData& item)
{
    using ResType = Optional<ResourceRestructuringData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM ResourceRestructuringManager WHERE ResourceRestructuringID = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.resourceRestructuringID));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static ResourceRestructuringData sqlRecordToResourceRestructuringDataItem(QSqlRecord& record)
{
    ResourceRestructuringData data;
    data.resourceRestructuringID = record.field("ResourceRestructuringID").value().toString();
    data.name = record.field("name").value().toString();
    data.workMode = (SystemWorkMode)record.field("workMode").value().toInt();
    data.isMaster = (MasterSlave)record.field("isMaster").value().toInt();
    data.desc = record.field("descs").value().toString();
    auto json = record.field("jsonData").value().toByteArray();
    json >> data.deviceMap;

    return data;
}

Optional<ResourceRestructuringDataList> SqlResourceRestructuringManager::selectItem()
{
    using ResType = Optional<ResourceRestructuringDataList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM ResourceRestructuringManager;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        ResourceRestructuringDataList itemList;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToResourceRestructuringDataItem(recordData);

            itemList.append(data);
        }

        return ResType(itemList);
    }
}
