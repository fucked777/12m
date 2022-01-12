#ifndef STATUSHISTORYSQL_H
#define STATUSHISTORYSQL_H

#include "StatusPersistenceMessageSerialize.h"
#include "Utility.h"
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QMutex>

struct StatusRecord
{
    int deviceID{0};
    QString id;
    QDateTime dateTime;
    QString json;
};

class StatusHistorySql
{
public:
    StatusHistorySql();

    /* 初始化数据库 需要重复调用 */
    OptionalNotValue initDB(QSqlDatabase& db);
    OptionalNotValue initDB();

    /* 插入状态数据 */
    OptionalNotValue insertStatus(QSqlDatabase& db, const QList<StatusRecord>& statusRecordList);
    OptionalNotValue insertStatus(const QList<StatusRecord>& statusRecordList);

    Optional<DeviceStatusLogDataList> search(QSqlDatabase& db, const QSet<int>& devices, const QSet<QString>& units,  QDateTime startTime, QDateTime endTime, const QSet<int>& modes, int currentPage, int pageSize);
    Optional<DeviceStatusLogDataList> search(const QSet<int>& devices, const QSet<QString>& units,  QDateTime startTime, QDateTime endTime, const QSet<int>& modes, int currentPage, int pageSize);

    int totalCount(QSqlDatabase& db, const QSet<int>& devices, const QSet<QString>& units, QDateTime startTime, QDateTime endTime, const QSet<int>& modes);
    int totalCount(const QSet<int>& devices, const QSet<QString>& units, QDateTime startTime, QDateTime endTime, const QSet<int>& modes);

    /* 获取设备状态的表名 */
    QString createTableName();
    QString createTableName(const QDateTime&);

    static QString createUuid();


private:
    QString m_connectName;
    QString m_tableName;
};

#endif  // STATUSHISTORYSQL_H
