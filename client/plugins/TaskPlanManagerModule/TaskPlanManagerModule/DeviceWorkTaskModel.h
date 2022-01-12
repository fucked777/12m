#ifndef DeviceWorkTaskModel_H
#define DeviceWorkTaskModel_H

#include "TaskPlanMessageDefine.h"
#include "Utility.h"
#include <QAbstractTableModel>
#include <QStandardItemModel>

class DeviceWorkTaskModelImpl;
class DeviceWorkTaskModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        SerialNumber = 0,  // 任务计划编号
        TaskPlanType,      // 任务计划类型
        TaskPlanSource,    // 任务计划来源
        TaskPlanCode,      // 任务计划代号
        CircleNo,          // 圈号
        WorkMode,          // 工作模式
        TaskStatus,        // 任务状态
        TaskStartTime,     // 任务开始时间
        TrackStartTime,    // 跟踪开始时间
        TrackEndTime,      // 跟踪结束时间
        TaskEndTime,       // 任务结束时间
        CreateTime,        // 计划提交时间
    };

public:
    explicit DeviceWorkTaskModel(QObject* parent = nullptr);
    ~DeviceWorkTaskModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Optional<DeviceWorkTask> getItem(const QModelIndex& index);
    void setTaskdata(const DeviceWorkTaskList& deviceWorkTaskdata);
    void deleteTaskdata(const DeviceWorkTask& deviceWorkTaskdata);
    bool judgeData(int row);
    void clear();

private:
    DeviceWorkTaskModelImpl* m_impl;
};

#endif  // SATELLITEMODEL_H
