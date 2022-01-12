#include "TcpStatusModel.h"

TcpStatusModel::TcpStatusModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex TcpStatusModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex TcpStatusModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int TcpStatusModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int TcpStatusModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void TcpStatusModel::setData(const QList<TcpStatus> &tcpStatusList)
{
    beginResetModel();
    mDataList = tcpStatusList;
    endResetModel();
}

QVariant TcpStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &tcpStatus = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return tcpStatus.id;
        case 1:
            return tcpStatus.bbeIndex;
        case 2:
            return tcpStatus.status;
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

void TcpStatusModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant TcpStatusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
