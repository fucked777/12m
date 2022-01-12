#include "SoftUnitHostModel.h"

SoftUnitHostModel::SoftUnitHostModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QVariant SoftUnitHostModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void SoftUnitHostModel::setHeaderData(const QStringList& headers) { m_headerList = headers; }

QModelIndex SoftUnitHostModel::index(int row, int column, const QModelIndex& parent) const
{
    // FIXME: Implement me!
    if (row < 0 || column < 0 || row >= m_dataList.size() || column >= m_headerList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex SoftUnitHostModel::parent(const QModelIndex& index) const
{
    // FIXME: Implement me!
    return QModelIndex();
}

int SoftUnitHostModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_dataList.size();

    // FIXME: Implement me!
}

int SoftUnitHostModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_headerList.size();

    // FIXME: Implement me!
}

void SoftUnitHostModel::setData(const QList<SoftUnitHost>& softUnitHostList)
{
    beginResetModel();
    m_dataList = softUnitHostList;
    endResetModel();
}

QVariant SoftUnitHostModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= m_dataList.size() || index.column() < 0 || index.column() >= m_headerList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const auto& softUnit = m_dataList.at(index.row());
        switch (index.column())
        {
        case 0: return softUnit.runCode;
        case 1: return softUnit.runStatus;
        default: break;
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QVariant();
}
