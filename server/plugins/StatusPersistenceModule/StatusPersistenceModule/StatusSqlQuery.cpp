#include "StatusSqlQuery.h"
#include "DBInterface.h"
#include "StatusPersistenceMessageSerialize.h"
#include "GlobalData.h"
#include "StatusHistorySql.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QUuid>
#include <QMutexLocker>


class StatusSqlQueryImpl
{
public:
    QList<StatusRecord> statusRecordList;
    StatusHistorySql sql;
    QMutex mutex;
};

const int loadNumberPerMore = 100;

StatusSqlQuery::StatusSqlQuery(QObject* parent)
    : QThread(parent)
    , m_impl(new StatusSqlQueryImpl)
{
}
StatusSqlQuery::~StatusSqlQuery()
{
    delete m_impl;
}

void StatusSqlQuery::run()
{
    auto connectName = DBInterface::createConnectName();
    int count =0;
    while(!isInterruptionRequested())
    {
        QThread::sleep(1);
        ++count;
        if(count < 5)
        {
            continue;
        }

        count=0;
        m_impl->mutex.lock();
        /* 每5s插入一次 */
        QList<StatusRecord> tempList;
        tempList.swap(m_impl->statusRecordList);
        m_impl->mutex.unlock();

        if(tempList.isEmpty())
        {
            continue;
        }

        //qWarning() << tempList.size();
        /* 获取配置是否入库 0不入库 1入库 */
        auto isInsert = GlobalData::getAutorunPolicyData("DeviceStatusInsertDB",0).toInt();
        if(isInsert != 1)
        {
            continue;
        }

        /* 创建表 */
        AutoCloseDBHelper closeHelper(connectName);
        {
            auto db = DBInterface::getDeviceStatusConnect(closeHelper());
            if (!db)
            {
                qWarning() << QString("状态入库连接数据库错误: %1").arg(db.msg());
                continue;
            }

            auto result = m_impl->sql.initDB(db.value());
            if(!result)
            {
                qWarning() << QString("状态入库: %1").arg(result.msg());
                continue;
            }

            result = m_impl->sql.insertStatus(tempList);
            if(!result)
            {
                qWarning() << QString("状态入库: %1").arg(result.msg());
                continue;
            }
        }
    }
}


void StatusSqlQuery::saveStatusResult(int device, const QString& json)
{
    QMutexLocker locker(&(m_impl->mutex));

    if(m_impl->statusRecordList.size() > 1000)
    {
        qWarning() << "状态过多丢弃状态数据";
        m_impl->statusRecordList.clear();
    }

    StatusRecord record;
    record.deviceID = device;
    record.id = StatusHistorySql::createUuid();
    record.dateTime = GlobalData::currentDateTime();
    record.json = json;


    m_impl->statusRecordList.append(record);
}
void StatusSqlQuery::stop()
{
    requestInterruption();
    quit();
    wait();
}
