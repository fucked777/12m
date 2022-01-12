#include "SqlSystemLogManager.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

#include "DBInterface.h"
#include "SystemLogMessageSerialize.h"

static bool queryLogData(const QString& sql, QSqlDatabase& dbImpl, QList<SystemLogData>& m_dataList)
{
    QSqlQuery sqlQuery = dbImpl.exec(sql);
    bool ret = sqlQuery.lastError().isValid();
    if (ret)
    {
        return false;
    }
    while (sqlQuery.next())
    {
        SystemLogData data;
        //查询数据行
        QSqlRecord recordData = sqlQuery.record();
        data.id = recordData.field("SysLogID").value().toString();
        data.createTime = recordData.field("SysTime").value().toString();
        data.level = LogLevel(recordData.field("SysLogLevel").value().toUInt());
        data.type = LogType(recordData.field("SysLogType").value().toUInt());
        data.module = recordData.field("SysModule").value().toString();
        data.userID = recordData.field("UserID").value().toString();
        data.context = recordData.field("SysLogInfo").value().toString();
        m_dataList.append(data);
    }
    return true;
}

SqlSystemLogManager::SqlSystemLogManager(QObject* parent)
    : QObject(parent)
{
    mDbConnectName = DBInterface::createConnectName();
}

SqlSystemLogManager::~SqlSystemLogManager() {}
OptionalNotValue SqlSystemLogManager::initDB()
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists SYSTEMLOG("
                      "SysLogID INT PRIMARY KEY AUTO_INCREMENT ,"
                      "SysTime bigint                             ,"
                      "SysLogLevel int                            ,"
                      "SysLogType int                             ,"
                      "SysModule blob                             ,"
                      "UserID varchar(64)                         ,"
                      "SysLogInfo blob);";
        //                      "CREATE UNIQUE INDEX time ON SYSTEMLOG(SysTime);";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            qWarning() << "系统日志数据库初始化失败,错误：" << lastError.text();
        }
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}

Optional<int> SqlSystemLogManager::totalNumberOfQueries(quint64 beginTime, quint64 endTime, int logLevel)
{
    using ResType = Optional<int>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql;
        if (logLevel < 0)
        {
            sql = QString("SELECT  COUNT(*) FROM SYSTEMLOG WHERE SysTime BETWEEN '%1' AND '%2';");
            sql = sql.arg(beginTime).arg(endTime);
        }
        else
        {
            sql = QString("SELECT  COUNT(*) FROM SYSTEMLOG WHERE SysTime BETWEEN '%1' AND '%2' AND SysLogLevel='%3';");
            sql = sql.arg(beginTime).arg(endTime).arg(logLevel);
        }

        int totalCount = -1;
        QSqlQuery query = db->exec(sql);
        if (query.next())
        {
            totalCount = query.value(0).toInt();
        }

        auto lastError = query.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(totalCount);
    }
}

Optional<SystemLogACK> SqlSystemLogManager::logConditionQuery(quint64 beginTime, quint64 endTime, int logLevel, int currentPage, int pageSize)
{
    using ResType = Optional<SystemLogACK>;
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql;
        if (logLevel < 0)
        {
            sql = "SELECT * FROM SYSTEMLOG WHERE SysTime BETWEEN '%1' AND '%2' order by SysTime desc limit %3,%4;";
            sql = sql.arg(beginTime).arg(endTime).arg((currentPage - 1) * pageSize).arg(pageSize);
        }
        else
        {
            sql = "SELECT * FROM SYSTEMLOG WHERE SysTime BETWEEN '%1' AND '%2'  AND SysLogLevel = %3  order by SysTime desc limit %4,%5;";
            sql = sql.arg(beginTime).arg(endTime).arg(logLevel).arg((currentPage - 1) * pageSize).arg(pageSize);
        }

        SystemLogACK ack;
        QList<SystemLogData> dataList;
        queryLogData(sql, db.value(), dataList);

        JsonWriter writer;
        writer& dataList;
        ack.dataList = dataList;
        ack.totalCount = dataList.size();
        ack.currentPage = currentPage;
        return ResType(ack);
    }
}

void SqlSystemLogManager::insertSysLog(const SystemLogData& pSystemLog)
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            qWarning() << "getShortConnect" << __FUNCTION__ << db.msg();
            return;
        }

        QString sqlTemplate = R"(INSERT INTO SYSTEMLOG (SysTime,SysLogLevel,SysLogType,SysModule,UserID,SysLogInfo)
                            VALUES (?,?,?,?,?,?);)";
        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sqlTemplate);
        //sqlQuery.bindValue(0, pSystemLog.id);
        sqlQuery.bindValue(0, pSystemLog.createTime);
        sqlQuery.bindValue(1, static_cast<quint32>(pSystemLog.level));
        sqlQuery.bindValue(2, static_cast<quint32>(pSystemLog.type));
        sqlQuery.bindValue(3, pSystemLog.module.toUtf8());
        sqlQuery.bindValue(4, pSystemLog.userID);
        sqlQuery.bindValue(5, pSystemLog.context.toUtf8());
        sqlQuery.exec();
    }
}

void SqlSystemLogManager::insertSysLog(const QList<SystemLogData>& pSystemLog)
{
    AutoCloseDBHelper closeHelper(mDbConnectName);
    {
        auto db = DBInterface::getShortConnect(mDbConnectName);
        if (!db)
        {
            qWarning() << "getShortConnect" << __FUNCTION__ << db.msg();
            return;
        }
        QString sqlTemplate = R"(INSERT INTO SYSTEMLOG (SysTime,SysLogLevel,SysLogType,SysModule,UserID,SysLogInfo)
                            VALUES (?,?,?,?,?,?);)";

        db->transaction();
        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sqlTemplate);
        for (auto& item : pSystemLog)
        {
            sqlQuery.bindValue(0, item.createTime);
            sqlQuery.bindValue(1, static_cast<quint32>(item.level));
            sqlQuery.bindValue(2, static_cast<quint32>(item.type));
            sqlQuery.bindValue(3, item.module.toUtf8());
            sqlQuery.bindValue(4, item.userID);
            sqlQuery.bindValue(5, item.context.toUtf8());
            sqlQuery.exec();
        }
        db->commit();
    }
}
