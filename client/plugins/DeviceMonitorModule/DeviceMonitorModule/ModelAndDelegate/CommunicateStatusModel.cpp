#include "CommunicateStatusModel.h"

CommunicateStatusModel::CommunicateStatusModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex CommunicateStatusModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex CommunicateStatusModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int CommunicateStatusModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int CommunicateStatusModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void CommunicateStatusModel::setData(const QList<CommunicateStatus> &commStatusList)
{
    beginResetModel();
    mDataList = commStatusList;
    endResetModel();
}

QVariant CommunicateStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &commStatus = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return commStatus.portId;
        case 1:
            return commStatus.portCode;
        case 2:
            return commStatus.bId;
        case 3:
            return commStatus.dataCode;
        case 4:
            return commStatus.mId;
        case 5:
            return commStatus.uac;
        case 6:
            return commStatus.framesNum;
        case 7:
            return commStatus.dir;
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

void CommunicateStatusModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant CommunicateStatusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
