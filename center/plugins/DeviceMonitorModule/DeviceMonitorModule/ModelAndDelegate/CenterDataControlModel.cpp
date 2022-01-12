#include "CenterDataControlModel.h"

CenterDataControlModel::CenterDataControlModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex CenterDataControlModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex CenterDataControlModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int CenterDataControlModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mDataList.size();
}

int CenterDataControlModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mHeaderList.size();
}

void CenterDataControlModel::setData(const QList<CenterDataControl> &centerDataControlList)
{
    beginResetModel();
    mDataList = centerDataControlList;
    endResetModel();
}

QVariant CenterDataControlModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &centerDataControl = mDataList.at(index.row());
        switch (index.column()) {
        case 0:
            return centerDataControl.id;
        case 1:
            return centerDataControl.code;
        case 2:
            return centerDataControl.isUing;
        case 3:
            return centerDataControl.isSendTtcFirstRouter;
        case 4:
            return centerDataControl.isSendLinkFirstRouter;
        case 5:
            return centerDataControl.isSendTtcSecondRouter;
        case 6:
            return centerDataControl.isSendLinkSecondRouter;
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

void CenterDataControlModel::setHeaderData(const QStringList &headers)
{
    mHeaderList = headers;
}

QVariant CenterDataControlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
