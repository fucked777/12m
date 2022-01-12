#include "DataTransferTaskModel.h"
#include "ExtendedConfig.h"
#include "TaskPlanMessageSerialize.h"
#include <QDebug>

struct DataTransferTaskModelShowData
{
    DataTranWorkTask dataTranWorkTask;
    QList<QString> data;
};

class DataTransferTaskModelImpl
{
public:
    QStringList headList;
    QList<DataTransferTaskModelShowData> dtTaskdata;
};

DataTransferTaskModel::DataTransferTaskModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new DataTransferTaskModelImpl)
{
    m_impl->headList /*<< "序号"*/
        << "计划编号"
        << "计划类型"
        << "来源"
        << "数传中心"
        << "优先级"
        //<< "跟踪接收计划"
        << "执行状态"
        << "执行天线"
        << "任务开始时间"
        << "事后传输时间"
        << "任务结束时间"
        << "计划提交时间"
        << "传输协议"
        //<< "传输通道"
        << "圈次内编号";
}
DataTransferTaskModel::~DataTransferTaskModel() { delete m_impl; }
QVariant DataTransferTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_impl->headList[section];
        }
    }
    }
    return QVariant();
}

int DataTransferTaskModel::rowCount(const QModelIndex& /* parent */) const { return m_impl->dtTaskdata.size(); }

int DataTransferTaskModel::columnCount(const QModelIndex& /* parent */) const { return m_impl->headList.size(); }

bool DataTransferTaskModel::judgeData(int row) { return (row >= 0 && row <= m_impl->dtTaskdata.size()); }

QVariant DataTransferTaskModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();  //行号
    if (row < 0 || row >= rowCount())
    {
        return QVariant();
    }
    int colnum = index.column();
    auto& data = m_impl->dtTaskdata.at(row);

    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return data.data.at(colnum);
        break;
    }
    case Qt::ForegroundRole:
    {
        switch (colnum)
        {
        case TaskStatus:
        {
            return TaskPlanHelper::taskStatusToColor(data.dataTranWorkTask.m_task_status);
        }
        }
        return QColor(Qt::black);
    }
    }

    return QVariant();
}

Qt::ItemFlags DataTransferTaskModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
void DataTransferTaskModel::clear()
{
    beginResetModel();
    m_impl->dtTaskdata.clear();
    endResetModel();
}
void DataTransferTaskModel::setTaskdata(const DataTranWorkTaskList& dataTranWorkTask)
{
    QList<DataTransferTaskModelShowData> dtTaskdata;
    for (auto& item : dataTranWorkTask)
    {
        DataTransferTaskModelShowData tempData;
        tempData.dataTranWorkTask = item;

        tempData.data << item.m_plan_serial_number;
        tempData.data << TaskPlanHelper::dataTransModeToString(item.m_working_mode);
        tempData.data << TaskPlanHelper::taskSourceToString(item.m_source);
        tempData.data << item.m_datatrans_center;
        tempData.data << TaskPlanHelper::dataTransPriorityToString(item.m_priority);
        tempData.data << TaskPlanHelper::taskStatusToString(item.m_task_status);
        tempData.data << ExtendedConfig::curTKID();
        tempData.data << Utility::dateTimeToStr(item.m_task_start_time);
        tempData.data << Utility::dateTimeToStr(item.m_transport_time);
        tempData.data << Utility::dateTimeToStr(item.m_task_end_time);
        tempData.data << Utility::dateTimeToStr(item.m_createTime);
        tempData.data << TaskPlanHelper::centerProtocolToDesc(item.m_center_protocol);
        tempData.data << QString::number(item.m_ring_num);
        dtTaskdata << tempData;

        Q_ASSERT(tempData.data.size() == columnCount());
    }

    beginResetModel();
    m_impl->dtTaskdata = dtTaskdata;
    std::sort(m_impl->dtTaskdata.begin(), m_impl->dtTaskdata.end(),
              [](const DataTransferTaskModelShowData& data1, const DataTransferTaskModelShowData& data2) {
                  if (data1.dataTranWorkTask.m_task_status == data2.dataTranWorkTask.m_task_status)
                  {
                      return data1.dataTranWorkTask.m_plan_serial_number < data2.dataTranWorkTask.m_plan_serial_number;
                  }
                  else
                  {
                      return data1.dataTranWorkTask.m_task_status > data2.dataTranWorkTask.m_task_status;
                  }
              });

    endResetModel();
}

void DataTransferTaskModel::deleteTaskdata(const DataTranWorkTask& dataTranWorkTask)
{
    for (auto iter = m_impl->dtTaskdata.begin(); iter != m_impl->dtTaskdata.end(); ++iter)
    {
        if (iter->dataTranWorkTask.m_uuid == dataTranWorkTask.m_uuid)
        {
            beginResetModel();
            m_impl->dtTaskdata.erase(iter);
            endResetModel();
            return;
        }
    }
}
Optional<DataTranWorkTask> DataTransferTaskModel::getItem(const QModelIndex& index)
{
    using ResType = Optional<DataTranWorkTask>;
    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(m_impl->dtTaskdata.at(row).dataTranWorkTask);
}
