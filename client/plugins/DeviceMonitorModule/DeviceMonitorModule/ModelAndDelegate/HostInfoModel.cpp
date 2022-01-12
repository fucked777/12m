#include "HostInfoModel.h"

HostInfoModel::HostInfoModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex HostInfoModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex HostInfoModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int HostInfoModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int HostInfoModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void HostInfoModel::setData(const QList<HostInfo> &hostInfoList)
{
    beginResetModel();
    mDataList = hostInfoList;
    endResetModel();
}

QVariant HostInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &hostInfo = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return hostInfo.IP;
        case 1:
            return hostInfo.name;
        case 2:
            return hostInfo.status;
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

void HostInfoModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant HostInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
