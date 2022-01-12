#include "SoftUnitModel.h"

SoftUnitModel::SoftUnitModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex SoftUnitModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex SoftUnitModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int SoftUnitModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int SoftUnitModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void SoftUnitModel::setData(const QList<SoftUnit> &softUnitList)
{
    beginResetModel();
    mDataList = softUnitList;
    endResetModel();
}

QVariant SoftUnitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &softUnit = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return softUnit.id;
        case 1:
            return softUnit.code;
        case 2:
            return softUnit.status;
        case 3:
            return softUnit.detailNum;
        case 4:
            return softUnit.hostId;
        case 5:
            return softUnit.detailStatus;
        default:
            break;
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QVariant();
}

void SoftUnitModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant SoftUnitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
