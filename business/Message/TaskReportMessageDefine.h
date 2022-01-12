#ifndef TASKREPORTMESSAGEDEFINE_H
#define TASKREPORTMESSAGEDEFINE_H

#include <QMap>
#include <QString>

struct TaskReportTable
{
    QString uuid;              // 任务唯一ID  任务计划的ID
    QString createTime;        // 计划创建 时间
    QString preStartTime;      // 准备开始时间
    QString startTime;         // 任务开始时间
    QString endTime;           // 计划结束时间
    QString planSerialNumber;  // 计划流水号
    QString taskCode;          // 任务代号
};

using TaskReportTableMap = QMap<QString, TaskReportTable>;
using TaskReportTableList = QList<TaskReportTable>;

#endif  // TASKREPORTMESSAGEDEFINE_H
