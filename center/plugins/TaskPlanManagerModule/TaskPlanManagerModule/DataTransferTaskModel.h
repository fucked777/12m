#ifndef DATATRANSFERTASKMODEL_H
#define DATATRANSFERTASKMODEL_H

#include "TaskPlanMessageDefine.h"
#include "Utility.h"
#include <QAbstractTableModel>
#include <QStandardItemModel>

class DataTransferTaskModelImpl;
class DataTransferTaskModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        SerialNumber = 0,  // 任务计划编号
        PlanType,          // 计划类型
        TaskPlanSource,    // 任务计划来源
        DataTransCenter,   // 数传中心
        Priority,          // 优先级
        // TrackPlan,            // 跟踪接收计划
        TaskStatus,               // 任务状态
        Acu,                      // 执行天线
        TaskStartTime,            // 任务开始时间
        TransportTime,            // 事后传输时间
        TaskEndTime,              // 任务结束时间
        CreateTime,               // 计划提交时间
        DataTransCenterProtocol,  // 数传中心协议
        // TransChannel,             // 传输通道
        RingNum,  // 圈次内编号
    };

public:
    explicit DataTransferTaskModel(QObject* parent = nullptr);
    ~DataTransferTaskModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setTaskdata(const DataTranWorkTaskList& dataTranWorkTask);
    void deleteTaskdata(const DataTranWorkTask& dataTranWorkTask);

    Optional<DataTranWorkTask> getItem(const QModelIndex& index);

    bool judgeData(int row);
    void clear();

private:
    DataTransferTaskModelImpl* m_impl;
};

#endif  // DATATRANSFERTASKMODEL_H
