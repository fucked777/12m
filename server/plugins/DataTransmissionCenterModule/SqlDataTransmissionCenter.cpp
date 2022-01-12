#include "SqlDataTransmissionCenter.h"

#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

#include "CConverter.h"
#include "DBInterface.h"
#include "DataTransmissionCenterSerialize.h"

SqlDataTransmissionCenter::SqlDataTransmissionCenter(QObject* parent)
    : QObject(parent)
    , m_dbConnectName(DBInterface::createConnectName())
{
}

SqlDataTransmissionCenter::~SqlDataTransmissionCenter() {}

OptionalNotValue SqlDataTransmissionCenter::initDB()
{
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        QString sql = " create table if not exists DataTransmissionCenter("
                      "centerName                 varchar(256) primary key ,"
                      "centerIP                   varchar(64)              ,"
                      "centerXS                   varchar(64)              ,"
                      "plaintextDestPortPDXP      integer                  ,"
                      "ciphertextDestPortPDXP     integer                  ,"
                      "plaintextSrcPortPDXP       integer                  ,"
                      "ciphertextSrcPortPDXP      integer                  ,"
                      "plaintextDestPortFep       integer                  ,"
                      "ciphertextDestPortFep      integer                  ,"
                      "plaintextSrcPortFep        integer                  ,"
                      "ciphertextSrcPortFep       integer                  ,"
                      "plaintextDestPortFepRECP   integer                  ,"
                      "ciphertextDestPortFepRECP  integer                  ,"
                      "plaintextSrcPortFepRECP    integer                  ,"
                      "ciphertextSrcPortFepRECP   integer                  ,"
                      "descText                   varchar(512))            ;";

        auto sqlQuery = db->exec(sql);
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? OptionalNotValue(ErrorCode::SqlExecFailed, lastError.text()) : OptionalNotValue();
    }
}
static bool dataExist(const DataTransmissionCenterData& item, QSqlDatabase& db)
{
    auto sql = QString("SELECT * FROM DataTransmissionCenter where centerName='%1' ;").arg(item.centerName);
    QSqlQuery sqlQuery(db);
    /* 执行错误,认为存在了,不让插入 */
    if (!sqlQuery.exec(sql))
    {
        //        Error(ErrorCode::SqlExecFailed, sqlQuery.lastError().text());
        return true;
    }
    return sqlQuery.next();
}
Optional<DataTransmissionCenterData> SqlDataTransmissionCenter::insertItem(const DataTransmissionCenterData& raw)
{
    using ResType = Optional<DataTransmissionCenterData>;
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
            auto msg = QString("当前名称对应的数传中心已存在:%1").arg(item.centerName);
            return ResType(ErrorCode::DataExist, msg);
        }
        QString sql = "insert into DataTransmissionCenter("
                      "centerName                 ,"
                      "centerIP                   ,"
                      "centerXS                   ,"
                      "plaintextDestPortPDXP      ,"
                      "ciphertextDestPortPDXP     ,"
                      "plaintextSrcPortPDXP       ,"
                      "ciphertextSrcPortPDXP      ,"
                      "plaintextDestPortFep       ,"
                      "ciphertextDestPortFep      ,"
                      "plaintextSrcPortFep        ,"
                      "ciphertextSrcPortFep       ,"
                      "plaintextDestPortFepRECP   ,"
                      "ciphertextDestPortFepRECP  ,"
                      "plaintextSrcPortFepRECP    ,"
                      "ciphertextSrcPortFepRECP   ,"
                      "descText                   )"
                      "values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

        QSqlQuery sqlQuery(db.value());

        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.centerName);
        sqlQuery.bindValue(1, item.centerIP);
        sqlQuery.bindValue(2, item.centerXS);
        sqlQuery.bindValue(3, item.plaintextDestPortPDXP);
        sqlQuery.bindValue(4, item.ciphertextDestPortPDXP);
        sqlQuery.bindValue(5, item.plaintextSrcPortPDXP);
        sqlQuery.bindValue(6, item.ciphertextSrcPortPDXP);
        sqlQuery.bindValue(7, item.plaintextDestPortFep);
        sqlQuery.bindValue(8, item.ciphertextDestPortFep);
        sqlQuery.bindValue(9, item.plaintextSrcPortFep);
        sqlQuery.bindValue(10, item.ciphertextSrcPortFep);
        sqlQuery.bindValue(11, item.plaintextDestPortFepRECP);
        sqlQuery.bindValue(12, item.ciphertextDestPortFepRECP);
        sqlQuery.bindValue(13, item.plaintextSrcPortFepRECP);
        sqlQuery.bindValue(14, item.ciphertextSrcPortFepRECP);
        sqlQuery.bindValue(15, item.desc);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<DataTransmissionCenterData> SqlDataTransmissionCenter::updateItem(const DataTransmissionCenterData& raw)
{
    using ResType = Optional<DataTransmissionCenterData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }
        QString sql = "update DataTransmissionCenter "
                      "set centerIP              = ?   ,"
                      "centerXS                  = ?   ,"
                      "plaintextDestPortPDXP     = ?   ,"
                      "ciphertextDestPortPDXP    = ?   ,"
                      "plaintextSrcPortPDXP      = ?   ,"
                      "ciphertextSrcPortPDXP     = ?   ,"
                      "plaintextDestPortFep      = ?   ,"
                      "ciphertextDestPortFep     = ?   ,"
                      "plaintextSrcPortFep       = ?   ,"
                      "ciphertextSrcPortFep      = ?   ,"
                      "plaintextDestPortFepRECP  = ?   ,"
                      "ciphertextDestPortFepRECP = ?   ,"
                      "plaintextSrcPortFepRECP   = ?   ,"
                      "ciphertextSrcPortFepRECP  = ?   ,"
                      "descText                  = ?   "
                      "where centerName          = ?   ;";

        QSqlQuery sqlQuery(db.value());
        sqlQuery.prepare(sql);
        sqlQuery.bindValue(0, item.centerIP);
        sqlQuery.bindValue(1, item.centerXS);
        sqlQuery.bindValue(2, item.plaintextDestPortPDXP);
        sqlQuery.bindValue(3, item.ciphertextDestPortPDXP);
        sqlQuery.bindValue(4, item.plaintextSrcPortPDXP);
        sqlQuery.bindValue(5, item.ciphertextSrcPortPDXP);
        sqlQuery.bindValue(6, item.plaintextDestPortFep);
        sqlQuery.bindValue(7, item.ciphertextDestPortFep);
        sqlQuery.bindValue(8, item.plaintextSrcPortFep);
        sqlQuery.bindValue(9, item.ciphertextSrcPortFep);
        sqlQuery.bindValue(10, item.plaintextDestPortFepRECP);
        sqlQuery.bindValue(11, item.ciphertextDestPortFepRECP);
        sqlQuery.bindValue(12, item.plaintextSrcPortFepRECP);
        sqlQuery.bindValue(13, item.ciphertextSrcPortFepRECP);
        sqlQuery.bindValue(14, item.desc);
        sqlQuery.bindValue(15, item.centerName);
        sqlQuery.exec();

        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
Optional<DataTransmissionCenterData> SqlDataTransmissionCenter::deleteItem(const DataTransmissionCenterData& raw)
{
    using ResType = Optional<DataTransmissionCenterData>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto item = raw;
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("DELETE FROM DataTransmissionCenter WHERE centerName = '%1';");
        auto sqlQuery = db->exec(sql.arg(item.centerName));
        auto lastError = sqlQuery.lastError();
        return lastError.isValid() ? ResType(ErrorCode::SqlExecFailed, lastError.text()) : ResType(item);
    }
}
static DataTransmissionCenterData sqlRecordToTCDataItem(QSqlRecord& record)
{
    DataTransmissionCenterData data;
    data.centerName = record.field("centerName").value().toString();
    data.centerIP = record.field("centerIP").value().toString();
    data.centerXS = record.field("centerXS").value().toString();
    data.plaintextDestPortPDXP = static_cast<quint16>(record.field("plaintextDestPortPDXP").value().toUInt());
    data.ciphertextDestPortPDXP = static_cast<quint16>(record.field("ciphertextDestPortPDXP").value().toUInt());
    data.plaintextSrcPortPDXP = static_cast<quint16>(record.field("plaintextSrcPortPDXP").value().toUInt());
    data.ciphertextSrcPortPDXP = static_cast<quint16>(record.field("ciphertextSrcPortPDXP").value().toUInt());
    data.plaintextDestPortFep = static_cast<quint16>(record.field("plaintextDestPortFep").value().toUInt());
    data.ciphertextDestPortFep = static_cast<quint16>(record.field("ciphertextDestPortFep").value().toUInt());
    data.plaintextSrcPortFep = static_cast<quint16>(record.field("plaintextSrcPortFep").value().toUInt());
    data.ciphertextSrcPortFep = static_cast<quint16>(record.field("ciphertextSrcPortFep").value().toUInt());
    data.plaintextDestPortFepRECP = static_cast<quint16>(record.field("plaintextDestPortFepRECP").value().toUInt());
    data.ciphertextDestPortFepRECP = static_cast<quint16>(record.field("ciphertextDestPortFepRECP").value().toUInt());
    data.plaintextSrcPortFepRECP = static_cast<quint16>(record.field("plaintextSrcPortFepRECP").value().toUInt());
    data.ciphertextSrcPortFepRECP = static_cast<quint16>(record.field("ciphertextSrcPortFepRECP").value().toUInt());
    data.desc = record.field("descText").value().toString();
    return data;
}
Optional<DataTransmissionCenterMap> SqlDataTransmissionCenter::selectItem()
{
    using ResType = Optional<DataTransmissionCenterMap>;
    AutoCloseDBHelper closeHelper(m_dbConnectName);
    {
        auto db = DBInterface::getShortConnect(m_dbConnectName);
        if (!db)
        {
            return ResType(db.errorCode(), db.msg());
        }

        QString sql("SELECT * FROM DataTransmissionCenter;");
        auto sqlQuery = db->exec(sql);

        auto lastError = sqlQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::SqlExecFailed, lastError.text());
        }

        DataTransmissionCenterMap itemMap;
        while (sqlQuery.next())
        {
            auto recordData = sqlQuery.record();
            auto data = sqlRecordToTCDataItem(recordData);
            itemMap[data.centerName] = data;
        }

        return ResType(itemMap);
    }
}
