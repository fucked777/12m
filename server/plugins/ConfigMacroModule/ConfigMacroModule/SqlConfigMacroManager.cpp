#include "SqlConfigMacroManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "ConfigMacroMessageSerialize.h"
#include "DBInterface.h"

SqlConfigMacroManager::SqlConfigMacroManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlConfigMacroManager::~SqlConfigMacroManager() {}
OptionalNotValue SqlConfigMacroManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS ConfigMacroManager("
                      "configMacroID VARCHAR(100) PRIMARY KEY, "
                      "configName VARCHAR(50), "
                      "workModeDesc text, "
                      "stationID VARCHAR(50), "
                      "isMaster INT, "
                      "describ VARCHAR(100), "
                      "jsonData BLOB);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "配置宏数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

static bool dataExist(const ConfigMacroData& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM ConfigMacroManager where configName='%1' ;").arg(item.configMacroName);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}

Optional<ConfigMacroData> SqlConfigMacroManager::insertItem(const ConfigMacroData& bak)
{
    auto item = bak;
    using ResType = Optional<ConfigMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        /* 首先判断数据是否存在 */
        if (dataExist(item, db.value()))
        {
            auto msg = QString("当前配置宏已存在:%1").arg(item.configMacroName);
            return ResType(ErrorCode::DataExist, msg);
        }
        item.configMacroID = QUuid::createUuid().toString();

        QString sql = "INSERT INTO ConfigMacroManager (configMacroID, configName , workModeDesc, stationID, isMaster , describ , jsonData)"
                      "VALUES (?,?, ?, ?, ?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.configMacroID);
        sqlQuery.bindValue(1, item.configMacroName);
        sqlQuery.bindValue(2, item.workModeDesc);
        sqlQuery.bindValue(3, item.stationID);
        sqlQuery.bindValue(4, static_cast<int>(item.isMaster));
        sqlQuery.bindValue(5, item.desc);
        QByteArray json;
        json << item.configMacroCmdModeMap;
        sqlQuery.bindValue(6, json);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ConfigMacroData> SqlConfigMacroManager::updateItem(const ConfigMacroData& bak)
{
    auto item = bak;
    using ResType = Optional<ConfigMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        if (item.configMacroID.isEmpty() || item.configMacroName.isEmpty())
        {
            return ResType(ErrorCode::NotFound, "当前配置宏不存在更新配置宏失败");
        }

        QString sql = QString("UPDATE ConfigMacroManager SET "
                              "configName = ?, "
                              "workModeDesc = ?, "
                              "stationID = ?, "
                              "isMaster = ?, "
                              "describ = ? , "
                              "jsonData = ? "
                              "WHERE configMacroID = ?;");

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, item.configMacroName);
        sqlQuery.bindValue(1, item.workModeDesc);
        sqlQuery.bindValue(2, item.stationID);
        sqlQuery.bindValue(3, static_cast<int>(item.isMaster));
        sqlQuery.bindValue(4, item.desc);
        QByteArray json;
        json << item.configMacroCmdModeMap;
        sqlQuery.bindValue(5, json);
        sqlQuery.bindValue(6, item.configMacroID);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<ConfigMacroData> SqlConfigMacroManager::deleteItem(const ConfigMacroData& item)
{
    using ResType = Optional<ConfigMacroData>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM ConfigMacroManager WHERE configMacroID = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.configMacroID));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static ConfigMacroData sqlRecordToConfigMacroDataItem(QSqlRecord& record)
{
    ConfigMacroData data;
    data.configMacroID = record.field("configMacroID").value().toString();
    data.configMacroName = record.field("configName").value().toString();
    data.workModeDesc = record.field("workModeDesc").value().toString();
    data.stationID = record.field("stationID").value().toString();
    data.isMaster = (MasterSlave)record.field("isMaster").value().toInt();
    data.desc = record.field("describ").value().toString();
    auto json = record.field("jsonData").value().toByteArray();
    json >> data.configMacroCmdModeMap;
    return data;
}

Optional<ConfigMacroDataList> SqlConfigMacroManager::selectItem()
{
    using ResType = Optional<ConfigMacroDataList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM ConfigMacroManager;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        ConfigMacroDataList itemList;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToConfigMacroDataItem(recordData);

            itemList.append(data);
        }

        return ResType(itemList);
    }
}
