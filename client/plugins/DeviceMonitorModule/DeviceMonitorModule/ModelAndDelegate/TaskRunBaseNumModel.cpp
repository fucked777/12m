#include "TaskRunBaseNumModel.h"

TaskRunBaseNumModel::TaskRunBaseNumModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QVariant TaskRunBaseNumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    if (section < 0 || section >= m_headerList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
            return m_headerList.at(section);
    }

    return QVariant();
}

void TaskRunBaseNumModel::setHeaderData(const QStringList& headers) { m_headerList = headers; }

QModelIndex TaskRunBaseNumModel::index(int row, int column, const QModelIndex& parent) const
{
    // FIXME: Implement me!
    if (row < 0 || column < 0 || row >= m_dataList.size() || column >= m_headerList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex TaskRunBaseNumModel::parent(const QModelIndex& index) const
{
    // FIXME: Implement me!
    return QModelIndex();
}

int TaskRunBaseNumModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_dataList.size();

    // FIXME: Implement me!
}

int TaskRunBaseNumModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_headerList.size();

    // FIXME: Implement me!
}

void TaskRunBaseNumModel::setData(const QList<TaskRunBaseNum>& taskRunBaseList)
{
    beginResetModel();
    m_dataList = taskRunBaseList;
    endResetModel();
}

QVariant TaskRunBaseNumModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= m_dataList.size() || index.column() < 0 || index.column() >= m_headerList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const auto& taskRunBaseNum = m_dataList.at(index.row());
        switch (index.column())
        {
        case 0: return taskRunBaseNum.taskRunBaseBZ;
        default: break;
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QVariant();
}
