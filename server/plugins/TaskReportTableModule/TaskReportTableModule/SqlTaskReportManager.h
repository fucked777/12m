#ifndef SQLTaskReportMANAGER_H
#define SQLTaskReportMANAGER_H

#include "Utility.h"
#include <QObject>

struct DeviceWorkTask;
using DeviceWorkTaskList = QList<DeviceWorkTask>;
struct TaskReportCondition;
struct TaskReportTable;
using TaskReportTableMap = QMap<QString, TaskReportTable>;
using TaskReportTableList = QList<TaskReportTable>;

class SqlTaskReportManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlTaskReportManager(QObject* parent = nullptr);
    ~SqlTaskReportManager();

public:
    //对表TaskReportTable的创建、增删查改等基本操作
    OptionalNotValue initDB();
    Optional<TaskReportTableList> getAllDataInfo();
    Optional<TaskReportTable> insertData(const TaskReportTable& data);
    Optional<TaskReportTable> deleteData(const TaskReportTable& data);
    Optional<TaskReportTable> updataData(const TaskReportTable& data);
    Optional<TaskReportTableList> getDataByUUID(QString uuid);

private:
    QString mDbConnectName;
};

#endif  // SQLTaskReportMANAGER_H
