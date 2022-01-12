#include "SqlUserManagerManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "DBInterface.h"
#include "UserManagerMessageSerialize.h"

SqlUserManagerManager::SqlUserManagerManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlUserManagerManager::~SqlUserManagerManager() {}
OptionalNotValue SqlUserManagerManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "CREATE TABLE IF NOT EXISTS UserManager("
                      "userID VARCHAR(100), "
                      "name VARCHAR(50) PRIMARY KEY, "
                      "password VARCHAR(20), "
                      "identity VARCHAR(10), "
                      "loginTime VARCHAR(30), "
                      "isOnline VARCHAR(10));";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "用户管理数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<User> SqlUserManagerManager::userRegister(const User& user)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = QString("SELECT count(*) FROM UserManager WHERE name = '%1';").arg(user.name);

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();

        sqlQuery.next();

        int count = sqlQuery.record().value(0).toInt();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }
        else if (0 != count)
        {
            return ResType(ErrorCode::NotFound, QString("数据库中已注册用户:%1!").arg(user.name).arg(user.password));
        }
        else
        {
            QString sql = "INSERT INTO UserManager (userID, name, password, identity, loginTime, isOnline)"
                          "VALUES (?,?,?,?,?,?);";

            QSqlQuery sqlQuery(db.value());

            sqlQuery.prepare(sql);
            sqlQuery.bindValue(0, user.userID);
            sqlQuery.bindValue(1, user.name);
            sqlQuery.bindValue(2, user.password);
            sqlQuery.bindValue(3, user.identity);
            sqlQuery.bindValue(4, user.loginTime);
            sqlQuery.bindValue(5, user.isOnline);

            sqlQuery.exec();

            auto lastError = sqlQuery.lastError();
            return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(user);
        }
    }
}

Optional<User> SqlUserManagerManager::modifyUser(const User& user)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = QString("UPDATE UserManager SET "
                              "name = ?, "
                              "password = ?, "
                              "identity = ?, "
                              "loginTime = ? , "
                              "isOnline = ? "
                              "WHERE userID = ?;");

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);

        sqlQuery.bindValue(0, user.name);
        sqlQuery.bindValue(1, user.password);
        sqlQuery.bindValue(2, user.identity);
        sqlQuery.bindValue(3, user.loginTime);
        sqlQuery.bindValue(4, user.isOnline);
        sqlQuery.bindValue(5, user.userID);

        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(user);
    }
}

Optional<User> SqlUserManagerManager::deleteUser(const User& user)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM UserManager WHERE userID = '%1';");
        auto sqlQuery = db->exec(sql.arg(user.userID));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(user);
    }
}

static User sqlRecordToUser(QSqlRecord& record)
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

Optional<UserList> SqlUserManagerManager::getUserList()
{
    using ResType = Optional<UserList>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = ("SELECT * FROM UserManager;");

        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        UserList userlist;
        while (sqlQuery.next())
        {
            //查询数据行
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToUser(recordData);

            userlist.append(data);
        }

        return ResType(userlist);
    }
}

Optional<bool> SqlUserManagerManager::defaultUserRegister(const User& user)
{
    using ResType = Optional<bool>;
    bool result = false;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = QString("SELECT count(*) FROM UserManager WHERE name = '%1';").arg(user.name);

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();

        sqlQuery.next();

        int count = sqlQuery.record().value(0).toInt();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }
        else if (0 != count)
        {
            return ResType(result);
        }
        else
        {
            QString sql = "INSERT INTO UserManager (userID, name, password, identity, loginTime, isOnline)"
                          "VALUES (?,?,?,?,?,?);";

            QSqlQuery sqlQuery(db.value());

            sqlQuery.prepare(sql);
            sqlQuery.bindValue(0, user.userID);
            sqlQuery.bindValue(1, user.name);
            sqlQuery.bindValue(2, user.password);
            sqlQuery.bindValue(3, user.identity);
            sqlQuery.bindValue(4, user.loginTime);
            sqlQuery.bindValue(5, user.isOnline);

            sqlQuery.exec();

            auto lastError = sqlQuery.lastError();
            result = true;
            return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(result);
        }
    }
}

Optional<User> SqlUserManagerManager::userLogin(const User& user)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql = QString("SELECT count(*) FROM UserManager WHERE name = '%1' and password = '%2';").arg(user.name).arg(user.password);

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();

        sqlQuery.next();

        int count = sqlQuery.record().value(0).toInt();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }
        else if (count == 0)
        {
            return ResType(ErrorCode::NotFound, QString("数据库中不存在用户名:%1,密码:%2信息").arg(user.name).arg(user.password));
        }
        else
        {
            sqlQuery.next();
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToUser(recordData);
            QString userId = data.userID;

            sql = QString("UPDATE UserManager SET "
                          "loginTime = '%1', "
                          "isOnline = '1' "
                          "WHERE name = '%2';")
                      .arg(user.loginTime)
                      .arg(user.name);
            QSqlQuery sqlUpdate(db.value());
            sqlUpdate.prepare(sql);
            sqlUpdate.exec();

            sql = QString("SELECT * FROM UserManager WHERE name = '%1' and password = '%2';").arg(user.name).arg(user.password);

            auto sqlQuery = db->exec(sql);

            auto lastError = sqlQuery.lastError();
            if (lastError.isValid())
            {
                return ResType(ErrorCode::SqlExecFailed, lastError.text());
            }

            User loginuser;
            while (sqlQuery.next())
            {
                //查询数据行
                recordData = sqlQuery.record();
                loginuser = sqlRecordToUser(recordData);
            }

            return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(loginuser);
        }
    }
}

Optional<User> SqlUserManagerManager::userLogout(const User& user)
{
    using ResType = Optional<User>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        //        QString sql = QString("SELECT count(*) FROM UserManager WHERE name = '%1' and password = '%2';").arg(user.name).arg(user.password);
        QString sql = QString("SELECT count(*) FROM UserManager WHERE name = '%1';").arg(user.name);

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();

        sqlQuery.next();

        int count = sqlQuery.record().value(0).toInt();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }
        else if (count == 0)
        {
            return ResType(ErrorCode::NotFound, QString("数据库中不存在用户名:%1,密码:%2信息").arg(user.name).arg(user.password));
        }
        else
        {
            sqlQuery.next();
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToUser(recordData);
            QString userId = data.userID;

            sql = QString("UPDATE UserManager SET "
                          "isOnline = '0' "
                          "WHERE name = '%1';")
                      .arg(user.name);
            QSqlQuery sqlUpdate(db.value());
            sqlUpdate.prepare(sql);
            sqlUpdate.exec();

            sql = QString("SELECT * FROM UserManager WHERE name = '%1' and password = '%2';").arg(user.name).arg(user.password);

            auto sqlQuery = db->exec(sql);

            auto lastError = sqlQuery.lastError();
            if (lastError.isValid())
            {
                return ResType(ErrorCode::SqlExecFailed, lastError.text());
            }

            User logoutuser;
            while (sqlQuery.next())
            {
                //查询数据行
                recordData = sqlQuery.record();
                logoutuser = sqlRecordToUser(recordData);
            }

            return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(logoutuser);
        }
    }
}
