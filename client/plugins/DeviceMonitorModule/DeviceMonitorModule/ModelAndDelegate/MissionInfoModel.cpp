#include "MissionInfoModel.h"

MissionInfoModel::MissionInfoModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex MissionInfoModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || row >= mDataList.size() || column >= mHeaderList.size())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QModelIndex MissionInfoModel::parent(const QModelIndex& child) const { return QModelIndex(); }

int MissionInfoModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mDataList.size(); }

int MissionInfoModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaderList.size(); }

void MissionInfoModel::setData(const QList<MissionInfo>& missionInfoList)
{
    beginResetModel();
    mDataList = missionInfoList;
    endResetModel();
}

QVariant MissionInfoModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mDataList.size() || index.column() < 0 || index.column() >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const auto& missionInfo = mDataList.at(index.row());
        switch (index.column())
        {
        case ID: return missionInfo.id;
        case UAC: return missionInfo.uac;
        case Center: return missionInfo.center;
        case Segment: return missionInfo.segment;
        case StarTime: return missionInfo.starTime;
        case StopTime: return missionInfo.stopTime;
        case RemainingTime: return missionInfo.remainingTime;
        case RunStatus: return missionInfo.runStatus;
        case GuideMode: return missionInfo.guideMode;
        case UpT0: return missionInfo.upT0;
        default: break;
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QVariant();
}

void MissionInfoModel::setHeaderData(const QStringList& headers) { mHeaderList = headers; }

QVariant MissionInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0 || section >= mHeaderList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
            return mHeaderList.at(section);
    }

    return QVariant();
}
