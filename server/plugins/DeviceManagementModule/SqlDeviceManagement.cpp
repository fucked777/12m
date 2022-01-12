#include "SqlDeviceManagement.h"
#include "DBInterface.h"
#include "DMMessageSerialize.h"
#include "GlobalData.h"
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <array>

SqlDeviceManagement::SqlDeviceManagement()
    : m_dbConnectName(DBInterface::createConnectName())
{
}

SqlDeviceManagement::~SqlDeviceManagement() {}
OptionalNotValue SqlDeviceManagement::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists DeviceManagement"
                      "( ID varchar(256) PRIMARY KEY ,DevID varchar(256) , DMType varchar(256) ,"
                      "createTime TIMESTAMP ,lastTime TIMESTAMP, content blob);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static bool dataExist(const DMDataItem& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM DeviceManagement where DMType='%1' and DevID='%2';").arg(item.type, item.devID);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}

Optional<DMDataItem> SqlDeviceManagement::insertItem(const DMDataItem& raw)
{
    using ResType = Optional<DMDataItem>;
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
            auto msg = QString("当前设备已存在:%1").arg(item.devID);
            return ResType(ErrorCode::DataExist, msg);
        }
        QString sql = "INSERT INTO DeviceManagement (ID,DMType,createTime, lastTime, content,DevID)"
                      "VALUES (?,?, ?, ?, ?,?);";

        QSqlQuery sqlQuery(db.value());

        item.createTime = GlobalData::currentDateTime();
        item.lastTime = item.createTime;
        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.id);
        sqlQuery.bindValue(1, item.type);
        sqlQuery.bindValue(2, item.createTime);
        sqlQuery.bindValue(3, item.lastTime);
        sqlQuery.bindValue(4, DMMessageSerializeHelper::joinDMDataItemParam(item));
        sqlQuery.bindValue(5, item.devID);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<DMDataItem> SqlDeviceManagement::updateItem(const DMDataItem& raw)
{
    using ResType = Optional<DMDataItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "UPDATE DeviceManagement SET lastTime = ?, DevID = ? , content = ? WHERE ID = ?;";

        item.lastTime = GlobalData::currentDateTime();
        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.lastTime);
        sqlQuery.bindValue(1, item.devID);
        sqlQuery.bindValue(2, DMMessageSerializeHelper::joinDMDataItemParam(item));
        sqlQuery.bindValue(3, item.id);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<DMDataItem> SqlDeviceManagement::deleteItem(const DMDataItem& raw)
{
    using ResType = Optional<DMDataItem>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM DeviceManagement WHERE ID = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.id));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
static DMDataItem sqlRecordToDMDataItem(QSqlRecord& record)
{
    DMDataItem data;
    data.id = record.field("ID").value().toString();
    data.createTime = record.field("createTime").value().toDateTime();
    data.lastTime = record.field("lastTime").value().toDateTime();
    data.type = record.field("DMType").value().toString();
    auto jsonData = record.field("content").value().toByteArray();
    DMMessageSerializeHelper::splitDMDataItemParam(jsonData, data);
    data.devID = record.field("DevID").value().toString();
    return data;
}
Optional<DMTypeMap> SqlDeviceManagement::selectItem(const DMQuery& query)
{
    using ResType = Optional<DMTypeMap>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql;
        if (query.type.isEmpty())
        {
            sql = ("SELECT * FROM DeviceManagement;");
        }
        else
        {
            sql = QString("SELECT * FROM DeviceManagement where DMType='%1';").arg(query.type);
        }

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DMTypeMap itemMap;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToDMDataItem(recordData);
            itemMap[data.type].append(data);
        }

        return ResType(itemMap);
    }
}
Optional<DMTypeMap> SqlDeviceManagement::selectItem()
{
    using ResType = Optional<QMap<QString, QList<DMDataItem>>>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM DeviceManagement;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DMTypeMap itemMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToDMDataItem(recordData);
            itemMap[data.type].append(data);
        }

        return ResType(itemMap);
    }
}
