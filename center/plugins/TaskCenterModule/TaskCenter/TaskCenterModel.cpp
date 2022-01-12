#include "TaskCenterModel.h"
#include "TCUtility.h"
#include "TaskCenterSerialize.h"
#include <QDebug>
#include <tuple>

using ModelDataList = QList<std::tuple<TaskCenterData, QStringList>>;
class TaskCenterModelImpl
{
public:
    QStringList headList{ TCUtility::modeHeader() };
    ModelDataList dataList;
};

TaskCenterModel::TaskCenterModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new TaskCenterModelImpl)
{
}
TaskCenterModel::~TaskCenterModel() { delete m_impl; }
void TaskCenterModel::addItem(const TaskCenterData& value)
{
    auto varList = TCUtility::data2Mode(value);
    if (varList.size() != m_impl->headList.size())
    {
        /* 这个算BUG wp?? */
        qWarning() << "任务中心数据和列表头数量不对应";
        return;
    }
    beginResetModel();
    m_impl->dataList << std::make_tuple(value, varList);
    endResetModel();
}
void TaskCenterModel::updateItem(const TaskCenterData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (TCUtility::dataCompare(std::get<0>(item), value))
        {
            beginResetModel();
            item = std::make_tuple(value, TCUtility::data2Mode(value));
            endResetModel();
            return;
        }
    }
}
void TaskCenterModel::deleteItem(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    auto row = index.row();
    if (row < 0 || row >= rowCount(index))
    {
        return;
    }
    beginResetModel();
    m_impl->dataList.removeAt(row);
    endResetModel();
}
void TaskCenterModel::deleteItem(const TaskCenterData& value)
{
    for (auto iter = m_impl->dataList.begin(); iter != m_impl->dataList.end(); ++iter)
    {
        if (TCUtility::dataCompare(std::get<0>(*iter), value))
        {
            beginResetModel();
            m_impl->dataList.erase(iter);
            endResetModel();
            return;
        }
    }
}
void TaskCenterModel::updateData(const TaskCenterList& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, TCUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void TaskCenterModel::updateData(const TaskCenterMap& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, TCUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void TaskCenterModel::updateData()
{
    beginResetModel();
    endResetModel();
}
bool TaskCenterModel::dataExist(const TaskCenterData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (TCUtility::dataCompare(std::get<0>(item), value))
        {
            return true;
        }
    }
    return false;
}
Optional<TaskCenterData> TaskCenterModel::getDataCenterItem(const QModelIndex& index)
{
    using ResType = Optional<TaskCenterData>;
    if (!index.isValid())
        return ResType(ErrorCode::InvalidArgument, "无效的索引");

    int row = index.row();  // 行号
    if (row < 0 || row >= rowCount(index))
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    return ResType(std::get<0>(m_impl->dataList.at(row)));
}
int TaskCenterModel::rowCount(const QModelIndex& /*parent*/) const { return m_impl->dataList.size(); }
int TaskCenterModel::columnCount(const QModelIndex& /*parent*/) const { return m_impl->headList.size(); }

QVariant TaskCenterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();  // 行号

    if (row < 0 || row >= rowCount(index))
        return QVariant();

    int colnum = index.column();
    auto& data = m_impl->dataList.at(row);
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return std::get<1>(data).at(colnum);
        break;
    }
    case Qt::CheckStateRole: return QVariant();
    }

    return QVariant();
}

QVariant TaskCenterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_impl->headList.at(section);
        }
    }
    default: return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags TaskCenterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
bool TaskCenterModel::judgeData(int row) { return (row >= 0 && row < rowCount()); }
