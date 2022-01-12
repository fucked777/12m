#include "ConfigMacroTableModel.h"

#include "ConfigMacroMessageDefine.h"

ConfigMacroTableModel::ConfigMacroTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "工作模式"
             << "站名"
             << "主备"
             << "描述";
}

ConfigMacroTableModel::~ConfigMacroTableModel() {}

int ConfigMacroTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mConfigMacroDatas.size(); }

int ConfigMacroTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant ConfigMacroTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();
    if (row < 0 || row >= mConfigMacroDatas.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const auto& data = mConfigMacroDatas.at(row);
        switch (col)
        {
        case WorkMode: return data.workModeDesc;
        case StationName: return data.stationID;
        case IsMaster: return data.isMaster == MasterSlave::Master ? "主" : "备";
        case Desc: return data.desc;
        }
    }
    break;
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    }

    return QVariant();
}

QVariant ConfigMacroTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags ConfigMacroTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void ConfigMacroTableModel::setConfigMacroData(const ConfigMacroDataList& list)
{
    beginResetModel();
    mConfigMacroDatas = list;
    endResetModel();
}

void ConfigMacroTableModel::addConfigMacroData(const ConfigMacroData& data)
{
    beginInsertRows(QModelIndex(), mConfigMacroDatas.size(), mConfigMacroDatas.size());
    mConfigMacroDatas.append(data);
    endInsertRows();
}

void ConfigMacroTableModel::deleteConfigMacroData(const QModelIndex& index)
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
    mConfigMacroDatas.removeAt(row);
    endRemoveRows();
}

void ConfigMacroTableModel::deleteConfigMacroData(const ConfigMacroData& data)
{
    for (auto iter = mConfigMacroDatas.begin(); iter != mConfigMacroDatas.end(); ++iter)
    {
        if (iter->configMacroID == data.configMacroID)
        {
            beginResetModel();
            mConfigMacroDatas.erase(iter);
            endResetModel();
            return;
        }
    }
}

void ConfigMacroTableModel::updateConfigMacroData(const ConfigMacroData& data)
{
    for (auto& configMacroData : mConfigMacroDatas)
    {
        if (configMacroData.configMacroID == data.configMacroID)
        {
            beginResetModel();
            configMacroData = data;
            beginResetModel();
            return;
        }
    }
}

Optional<ConfigMacroData> ConfigMacroTableModel::getConfigMacroData(const QModelIndex& index) const
{
    using ResType = Optional<ConfigMacroData>;
    if (!index.isValid())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(mConfigMacroDatas.at(row));
}
