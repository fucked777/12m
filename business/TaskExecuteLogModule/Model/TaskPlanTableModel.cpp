#include "TaskPlanTableModel.h"

#include "TaskPlanMessageDefine.h"
#include "TaskPlanMessageSerialize.h"
#include "Utility.h"
#include <QDateTime>

TaskPlanTableModel::TaskPlanTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "任务ID"
             << "创建时间"
             << "开始时间"
             << "结束时间"
             << "任务代号"
             << "任务类型"
             << "工作模式"
             << "是否校零"
             << "是否校相"
             << "主跟目标"
             //             << "跟踪开始时间"
             << "任务状态"
             << "计划位置";
}

TaskPlanTableModel::~TaskPlanTableModel() {}

int TaskPlanTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mDeviceWorkTasks.size(); }

int TaskPlanTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant TaskPlanTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();

    if (row < 0 || row >= mDeviceWorkTasks.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    auto& data = mDeviceWorkTasks.at(row);
    switch (role)
    {
    case Qt::DisplayRole:
    {
        DeviceWorkTaskTarget mainTarget;
        data.getMainTarget(mainTarget);

        switch (col)
        {
        case Uuid: return data.m_uuid;
        case CreateTime: return data.m_createTime.toString(DATETIME_DISPLAY_FORMAT2);
        case StartTime: return mainTarget.m_task_start_time.toString(DATETIME_DISPLAY_FORMAT2);
        case EndTime: return mainTarget.m_task_end_time.toString(DATETIME_DISPLAY_FORMAT2);
        case TaskPlanCode: return mainTarget.m_task_code;
        case TaskPlanType: return TaskPlanHelper::taskTypeToString(mainTarget.m_planType);
        case WorkMode: return TaskPlanHelper::planWorkModeToDesc(data.m_working_mode);
        case ZeroCorrection: return data.m_is_zero == TaskCalibration::CaliBrationOnce ? QString("是") : QString("否");
        case PhaseCorrection: return data.m_calibration == TaskCalibration::CaliBrationOnce ? QString("是") : QString("否");
        case MainTarget: return mainTarget.m_task_code;
        case TaskStatus: return TaskPlanHelper::taskStatusToString(data.m_task_status);
        case TaskPlanSource: return TaskPlanHelper::taskSourceToString(data.m_source);
        }
    }
    break;
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant TaskPlanTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return mHeaders.at(section);
        }
    }
    default: return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags TaskPlanTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void TaskPlanTableModel::setDeviceWorkTaskDatas(const DeviceWorkTaskList& list)
{
    beginResetModel();
    mDeviceWorkTasks = list;
    endResetModel();
}

Optional<DeviceWorkTask> TaskPlanTableModel::getDeviceWorkTaskData(const QModelIndex& index) const
{
    using ResType = Optional<DeviceWorkTask>;
    if (!index.isValid())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(mDeviceWorkTasks.at(row));
}
