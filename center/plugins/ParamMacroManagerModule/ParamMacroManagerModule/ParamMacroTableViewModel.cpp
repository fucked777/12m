#include "ParamMacroTableViewModel.h"

#include "ParamMacroMessage.h"

ParamMacroTableViewModel::ParamMacroTableViewModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "参数宏名称"
             << "工作模式"
             << "任务代号"
             << "参数宏描述";
}

ParamMacroTableViewModel::~ParamMacroTableViewModel() {}

int ParamMacroTableViewModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mParamMacroDatas.size(); }

int ParamMacroTableViewModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant ParamMacroTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();
    if (row < 0 || row >= mParamMacroDatas.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const auto& data = mParamMacroDatas.at(row);
        switch (col)
        {
        case Name: return data.name;
        case WorkMode: return data.workMode;
        case TaskCode: return data.taskCode;
        case Desc: return data.desc;
        }
    }
    break;
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    }

    return QVariant();
}

QVariant ParamMacroTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags ParamMacroTableViewModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void ParamMacroTableViewModel::setParamMacroData(const ParamMacroDataList& list)
{
    beginResetModel();
    mParamMacroDatas = list;
    endResetModel();
}

void ParamMacroTableViewModel::addParamMacroData(const ParamMacroData& data)
{
    beginInsertRows(QModelIndex(), mParamMacroDatas.size(), mParamMacroDatas.size());
    mParamMacroDatas.append(data);
    endInsertRows();
}

void ParamMacroTableViewModel::deleteParamMacroData(const QModelIndex& index)
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
    mParamMacroDatas.removeAt(row);
    endRemoveRows();
}

void ParamMacroTableViewModel::deleteParamMacroData(const ParamMacroData& data)
{
    for (auto iter = mParamMacroDatas.begin(); iter != mParamMacroDatas.end(); ++iter)
    {
        if (iter->taskCode == data.taskCode)
        {
            beginResetModel();
            mParamMacroDatas.erase(iter);
            endResetModel();
            return;
        }
    }
}

void ParamMacroTableViewModel::updateParamMacroData(const ParamMacroData& data)
{
    for (auto& configMacroData : mParamMacroDatas)
    {
        if (configMacroData.taskCode == data.taskCode)
        {
            beginResetModel();
            configMacroData = data;
            beginResetModel();
            return;
        }
    }
}

Optional<ParamMacroData> ParamMacroTableViewModel::getParamMacroData(const QModelIndex& index) const
{
    using ResType = Optional<ParamMacroData>;
    if (!index.isValid())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(mParamMacroDatas.at(row));
}
