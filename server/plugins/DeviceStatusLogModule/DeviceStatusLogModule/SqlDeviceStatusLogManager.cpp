#include "SqlDeviceStatusLogManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "DBInterface.h"
#include "DeviceStatusLogMessageSerialize.h"

SqlDeviceStatusLogManager::SqlDeviceStatusLogManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlDeviceStatusLogManager::~SqlDeviceStatusLogManager() {}
OptionalNotValue SqlDeviceStatusLogManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS DeviceStatusLog("
                      "id bigint PRIMARY KEY, "
                      "createTime VARCHAR(20), "
                      "deviceId VARCHAR(10), "
                      "unitId VARCHAR(2), "
                      "extensionMsg blob);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "设备状态日志数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<User> SqlDeviceStatusLogManager::insertItem(const User& item)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = "INSERT INTO DeviceStatusLog (userID, name, password, identity, loginTime, isOnline)"
                      "VALUES (?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.userID);
        sqlQuery.bindValue(1, item.name);
        sqlQuery.bindValue(2, item.password);
        sqlQuery.bindValue(3, item.identity);
        sqlQuery.bindValue(4, item.loginTime);
        sqlQuery.bindValue(5, item.isOnline);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

Optional<User> SqlDeviceStatusLogManager::deleteItem(const User& item)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM DeviceStatusLog WHERE userID = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.userID));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}

static User sqlRecordToUserItem(QSqlRecord& record)
{
    User data;
    data.userID = record.field("userID").value().toString();
    data.name = record.field("name").value().toString();
    data.password = record.field("password").value().toString();
    data.identity = record.field("identity").value().toString();
    data.loginTime = record.field("loginTime").value().toString();
    data.isOnline = record.field("isOnline").value().toString();

    return data;
}

Optional<UserList> SqlDeviceStatusLogManager::selectItem()
{
    using ResType = Optional<UserList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM DeviceStatusLog;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        UserList itemList;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToUserItem(recordData);

            itemList.append(data);
        }

        return ResType(itemList);
    }
}
