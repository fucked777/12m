#include "NetCardModel.h"

NetCardModel::NetCardModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex NetCardModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex NetCardModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int NetCardModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int NetCardModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void NetCardModel::setData(const QList<NetCardInfo> &netCardInfoList)
{
    beginResetModel();
    mDataList = netCardInfoList;
    endResetModel();
}

QVariant NetCardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const auto &netCardInfo = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return netCardInfo.IP;
        case 1:
            return netCardInfo.hostId;
        case 2:
            return netCardInfo.type;
        case 3:
            return netCardInfo.status;
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

void NetCardModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant NetCardModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
