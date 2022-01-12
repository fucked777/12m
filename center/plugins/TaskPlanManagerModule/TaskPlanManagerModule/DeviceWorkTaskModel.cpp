#include "DeviceWorkTaskModel.h"
#include "TaskPlanMessageSerialize.h"
#include <QDateTime>
#include <QDebug>
struct DeviceWorkTaskModelShowData
{
    DeviceWorkTask deviceWorkTask;
    QList<QString> data;
};

class DeviceWorkTaskModelImpl
{
public:
    QStringList headList;
    QList<DeviceWorkTaskModelShowData> deviceWorkTaskdata;
};

DeviceWorkTaskModel::DeviceWorkTaskModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new DeviceWorkTaskModelImpl)
{
    m_impl->headList << "计划编号"
                     << "计划类型"
                     << "来源"
                     << "任务代号"
                     << "圈号"
                     << "工作方式"
                     << "任务状态"
                     << "任务开始时间"
                     << "跟踪开始时间"
                     << "跟踪结束时间"
                     << "任务结束时间"
                     << "计划提交时间";
}
DeviceWorkTaskModel::~DeviceWorkTaskModel() { delete m_impl; }
QVariant DeviceWorkTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_impl->headList.at(section);
        }
    }
    }
    return QVariant();
}

int DeviceWorkTaskModel::rowCount(const QModelIndex& /* parent */) const { return m_impl->deviceWorkTaskdata.size(); }

int DeviceWorkTaskModel::columnCount(const QModelIndex& /* parent */) const { return m_impl->headList.size(); }

QVariant DeviceWorkTaskModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();  //行号
    if (row < 0 || row >= rowCount())
    {
        return QVariant();
    }

    int colnum = index.column();
    auto& data = m_impl->deviceWorkTaskdata.at(row);
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return data.data.at(colnum);
    }
    case Qt::ForegroundRole:
    {
        switch (colnum)
        {
        case TaskStatus:
        {
            return TaskPlanHelper::taskStatusToColor(data.deviceWorkTask.m_task_status);
        }
        }
        return QColor(Qt::black);
    }
    }
    return QVariant();
}

bool DeviceWorkTaskModel::judgeData(int row) { return (row >= 0 && row <= rowCount()); }

Qt::ItemFlags DeviceWorkTaskModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

Optional<DeviceWorkTask> DeviceWorkTaskModel::getItem(const QModelIndex& index)
{
    using ResType = Optional<DeviceWorkTask>;

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(m_impl->deviceWorkTaskdata.at(row).deviceWorkTask);
}
void DeviceWorkTaskModel::clear()
{
    beginResetModel();
    m_impl->deviceWorkTaskdata.clear();
    endResetModel();
}
void DeviceWorkTaskModel::setTaskdata(const DeviceWorkTaskList& deviceWorkTaskdata)
{
    QList<DeviceWorkTaskModelShowData> tempDeviceWorkTaskdata;
    for (auto& item : deviceWorkTaskdata)
    {
        DeviceWorkTaskModelShowData tempData;
        tempData.deviceWorkTask = item;

        DeviceWorkTaskTarget mainTarget;
        item.getMainTarget(mainTarget);

        tempData.data << item.m_plan_serial_number;
        tempData.data << TaskPlanHelper::taskTypeToString(mainTarget.m_planType);
        tempData.data << TaskPlanHelper::taskSourceToString(item.m_source);
        tempData.data << mainTarget.m_task_code;
        tempData.data << QString::number(mainTarget.m_circle_no);
        tempData.data << TaskPlanHelper::planWorkModeToDesc(item.m_working_mode);
        tempData.data << TaskPlanHelper::taskStatusToString(item.m_task_status);
        tempData.data << Utility::dateTimeToStr(item.m_startTime);
        tempData.data << Utility::dateTimeToStr(mainTarget.m_track_start_time);
        tempData.data << Utility::dateTimeToStr(mainTarget.m_track_end_time);
        tempData.data << Utility::dateTimeToStr(item.m_endTime);
        tempData.data << Utility::dateTimeToStr(item.m_createTime);

        tempDeviceWorkTaskdata << tempData;
        Q_ASSERT(tempData.data.size() == columnCount());
    }

    beginResetModel();
    m_impl->deviceWorkTaskdata = tempDeviceWorkTaskdata;
    std::sort(m_impl->deviceWorkTaskdata.begin(), m_impl->deviceWorkTaskdata.end(),
              [](const DeviceWorkTaskModelShowData& data1, const DeviceWorkTaskModelShowData& data2) {
                  if (data1.deviceWorkTask.m_task_status == data2.deviceWorkTask.m_task_status)
                  {
                      return data1.deviceWorkTask.m_plan_serial_number < data2.deviceWorkTask.m_plan_serial_number;
                  }
                  else
                  {
                      return data1.deviceWorkTask.m_task_status > data2.deviceWorkTask.m_task_status;
                  }
              });

    endResetModel();
}
void DeviceWorkTaskModel::deleteTaskdata(const DeviceWorkTask& deviceWorkTaskdata)
{
    for (auto iter = m_impl->deviceWorkTaskdata.begin(); iter != m_impl->deviceWorkTaskdata.end(); ++iter)
    {
        if (iter->deviceWorkTask.m_uuid == deviceWorkTaskdata.m_uuid)
        {
            beginResetModel();
            m_impl->deviceWorkTaskdata.erase(iter);
            endResetModel();
            return;
        }
    }
}
