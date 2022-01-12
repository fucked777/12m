#ifndef TaskPlanTableModel_H
#define TaskPlanTableModel_H

#include "Utility.h"
#include <QAbstractTableModel>

struct DeviceWorkTask;
using DeviceWorkTaskList = QList<DeviceWorkTask>;
using DeviceWorkTaskMap = QMap<QString, DeviceWorkTask>;

class TaskPlanTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        Uuid,             // 任务ID
        CreateTime,       // 创建时间
        StartTime,        // 开始时间
        EndTime,          // 结束时间
        TaskPlanCode,     // 任务计划代号
        TaskPlanType,     // 任务类型
        WorkMode,         // 工作模式
        ZeroCorrection,   // 是否校零
        PhaseCorrection,  // 是否校相
        MainTarget,       // 主跟目标
        TaskStatus,       // 任务状态
        TaskPlanSource,   // 任务计划来源
    };
    explicit TaskPlanTableModel(QObject* parent = nullptr);
    ~TaskPlanTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

    void setDeviceWorkTaskDatas(const DeviceWorkTaskList& list);
    Optional<DeviceWorkTask> getDeviceWorkTaskData(const QModelIndex& index) const;

private:
    QStringList mHeaders;
    DeviceWorkTaskList mDeviceWorkTasks;
};

#endif  // TaskPlanTableModel_H
