#include "SqlTaskCenter.h"

#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

#include "CConverter.h"
#include "DBInterface.h"
#include "TaskCenterSerialize.h"

SqlTaskCenter::SqlTaskCenter(QObject* parent)
    : QObject(parent)
    , m_dbConnectName(DBInterface::createConnectName())
{
}

SqlTaskCenter::~SqlTaskCenter() {}

OptionalNotValue SqlTaskCenter::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = "create table if not exists TaskCenter("
                      "uac varchar(256)     primary key     , "
                      "centername           varchar(256)    , "
                      "centercode           varchar(256)    , "
                      "frameformat          varchar(256)    , "
                      "diretype             varchar(256)    , "
                      "isusedlinkmonitor    integer         , "
                      "isused               integer         );";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static bool dataExist(const TaskCenterData& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM TaskCenter where uac='%1' ;").arg(item.uacAddr);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        // Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
Optional<TaskCenterData> SqlTaskCenter::insertItem(const TaskCenterData& raw)
{
    using ResType = Optional<TaskCenterData>;
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
            auto msg = QString("当前uac地址对应的任务中心已存在:%1").arg(item.uacAddr);
            return ResType(ErrorCode::DataExist, msg);
        }
        QString sql = "insert into TaskCenter("
                      "centername, "
                      "centercode, "
                      "frameformat, "
                      "diretype, "
                      "isusedlinkmonitor, "
                      "isused, "
                      "uac) "
                      "values (?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.centerName);
        sqlQuery.bindValue(1, item.centerCode);
        sqlQuery.bindValue(2, TCConvert::tcffToStr(item.frameFormat));
        sqlQuery.bindValue(3, TCConvert::tcdcToStr(item.direType));
        sqlQuery.bindValue(4, static_cast<quint32>(item.isUsedLinkMonitor));
        sqlQuery.bindValue(5, static_cast<quint32>(item.isUsed));
        sqlQuery.bindValue(6, item.uacAddr);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<TaskCenterData> SqlTaskCenter::updateItem(const TaskCenterData& raw)
{
    using ResType = Optional<TaskCenterData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = "update TaskCenter "
                      "set centername       = ?, "
                      "centercode           = ?, "
                      "frameformat          = ?, "
                      "diretype             = ?, "
                      "isusedlinkmonitor    = ?, "
                      "isused               = ?  "
                      "where uac            = ?; ";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.centerName);
        sqlQuery.bindValue(1, item.centerCode);
        sqlQuery.bindValue(2, TCConvert::tcffToStr(item.frameFormat));
        sqlQuery.bindValue(3, TCConvert::tcdcToStr(item.direType));
        sqlQuery.bindValue(4, static_cast<quint32>(item.isUsedLinkMonitor));
        sqlQuery.bindValue(5, static_cast<quint32>(item.isUsed));
        sqlQuery.bindValue(6, item.uacAddr);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<TaskCenterData> SqlTaskCenter::deleteItem(const TaskCenterData& raw)
{
    using ResType = Optional<TaskCenterData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM TaskCenter WHERE uac = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.uacAddr));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
static TaskCenterData sqlRecordToTCDataItem(QSqlRecord& record)
{
    TaskCenterData data;
    data.centerName = record.field("centerName").value().toString();
    data.centerCode = record.field("centerCode").value().toString();
    data.frameFormat = TCConvert::tcffFromStr(record.field("frameFormat").value().toString());
    data.direType = TCConvert::tcdcFromStr(record.field("direType").value().toString());
    data.isUsedLinkMonitor = record.field("isUsedLinkMonitor").value().toBool();
    data.isUsed = record.field("isUsed").value().toBool();
    data.uacAddr = record.field("UAC").value().toString();
    return data;
}
Optional<TaskCenterMap> SqlTaskCenter::selectItem()
{
    using ResType = Optional<TaskCenterMap>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM TaskCenter;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        TaskCenterMap itemMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToTCDataItem(recordData);
            itemMap[data.uacAddr] = data;
        }

        return ResType(itemMap);
    }
}
