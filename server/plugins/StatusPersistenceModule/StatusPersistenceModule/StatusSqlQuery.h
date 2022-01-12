#ifndef STATUSSQLQUERY_H
#define STATUSSQLQUERY_H

#include "StatusPersistenceMessageDefine.h"
#include "Utility.h"
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QMutex>

class StatusSqlQueryImpl;
class StatusSqlQuery : public QThread
{
public:
    StatusSqlQuery(QObject* parent = nullptr);
    ~StatusSqlQuery();

    void stop();
    void saveStatusResult(int device, const QString& json);

private:
    void run() override;

private:
    StatusSqlQueryImpl *m_impl;
};

#endif  // STATUSSQLQUERY_H
