#include "ResourceRestructuringTableModel.h"

ResourceRestructuringTableModel::ResourceRestructuringTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "名称"
             << "工作模式"
             << "主备"
             << "描述";
}

int ResourceRestructuringTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mResourceRestructuringDatas.size(); }

int ResourceRestructuringTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant ResourceRestructuringTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();
    if (row < 0 || row >= mResourceRestructuringDatas.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const auto& data = mResourceRestructuringDatas.at(row);
        switch (col)
        {
        case Name: return data.name;
        case WorkMode: return SystemWorkModeHelper::systemWorkModeToDesc(data.workMode);
        case IsMaster: return data.isMaster == MasterSlave::Master ? "主" : "备";
        case Desc: return data.desc;
        }
    }
    break;
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    }

    return QVariant();
}

QVariant ResourceRestructuringTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return mHeaders.at(section);
        }
    }
    default: return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags ResourceRestructuringTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void ResourceRestructuringTableModel::setDatas(const ResourceRestructuringDataList& list)
{
    beginResetModel();
    mResourceRestructuringDatas = list;
    endResetModel();
}

void ResourceRestructuringTableModel::addData(const ResourceRestructuringData& data)
{
    beginInsertRows(QModelIndex(), mResourceRestructuringDatas.size(), mResourceRestructuringDatas.size());
    mResourceRestructuringDatas.append(data);
    endInsertRows();
}

void ResourceRestructuringTableModel::deleteData(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    auto row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    mResourceRestructuringDatas.removeAt(row);
    endRemoveRows();
}

void ResourceRestructuringTableModel::deleteData(const ResourceRestructuringData& data)
{
    for (auto iter = mResourceRestructuringDatas.begin(); iter != mResourceRestructuringDatas.end(); ++iter)
    {
        if (iter->resourceRestructuringID == data.resourceRestructuringID)
        {
            beginResetModel();
            mResourceRestructuringDatas.erase(iter);
            endResetModel();
            return;
        }
    }
}

void ResourceRestructuringTableModel::updateData(const ResourceRestructuringData& data)
{
    for (auto& resourceRestructuringData : mResourceRestructuringDatas)
    {
        if (resourceRestructuringData.resourceRestructuringID == data.resourceRestructuringID)
        {
            beginResetModel();
            resourceRestructuringData = data;
            beginResetModel();
            return;
        }
    }
}

Optional<ResourceRestructuringData> ResourceRestructuringTableModel::getData(const QModelIndex& index) const
{
    using ResType = Optional<ResourceRestructuringData>;
    if (!index.isValid())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(mResourceRestructuringDatas.at(row));
}
