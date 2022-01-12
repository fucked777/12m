#ifndef SQLTASKCENTER_H
#define SQLTASKCENTER_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct TaskCenterData;
using TaskCenterMap = QMap<QString, TaskCenterData>;
using TaskCenterList = QList<TaskCenterData>;
class SqlTaskCenter : public QObject
{
    Q_OBJECT
public:
    explicit SqlTaskCenter(QObject* parent = nullptr);
    ~SqlTaskCenter();

public:
    OptionalNotValue initDB();
    Optional<TaskCenterData> insertItem(const TaskCenterData& item);
    Optional<TaskCenterData> updateItem(const TaskCenterData& item);
    Optional<TaskCenterData> deleteItem(const TaskCenterData& item);
    Optional<TaskCenterMap> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLTASKCENTER_H
