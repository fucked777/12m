#include "SqlDeviceStatusManager.h"
#include "DBInterface.h"
#include "StatusPersistenceMessageSerialize.h"
#include "StatusHistorySql.h"
#include "MessagePublish.h"
#include <QDateTime>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QUuid>

const int loadNumberPerMore = 200;

class SqlDeviceStatusManagerImpl
{
public:
    StatusHistorySql sql;
    bool hasQueryFinished{true};
};

SqlDeviceStatusManager::SqlDeviceStatusManager(QObject* parent)
    : QObject(parent)
    , m_impl(new SqlDeviceStatusManagerImpl)
{
}
SqlDeviceStatusManager::~SqlDeviceStatusManager()
{
    delete m_impl;
}

bool SqlDeviceStatusManager::queryFinished() { return m_impl->hasQueryFinished; }

void SqlDeviceStatusManager::searchLog(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time,
                                       const QDateTime& end_time, const QSet<int>& modes, int currentPage, int pageSize)
{
    m_impl->hasQueryFinished = false;
    auto result = m_impl->sql.search(devices, units, start_time, end_time, modes, currentPage, pageSize);
    m_impl->hasQueryFinished = true;

    if(!result)
    {
        SystemLogPublish::warningMsg(result.msg());
        return;
    }
    emit searchLogAck(result.value());
}

void SqlDeviceStatusManager::totalCount(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time,
                                        const QDateTime& end_time, const QSet<int>& modes, int currentPage)
{

    auto totalCount = m_impl->sql.totalCount(devices, units, start_time, end_time, modes);
    emit totalCountAck(totalCount, currentPage);
}
