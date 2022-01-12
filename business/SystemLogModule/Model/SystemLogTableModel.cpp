#include "SystemLogTableModel.h"

#include "SystemLogMessageDefine.h"
#include "SystemLogMessageSerialize.h"
#include <QDateTime>

SystemLogTableModel::SystemLogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders /*<< "日志ID"*/
        << "时间"
        << "日志等级"
        << "日志类型"
        << "模块"
        << "用户ID"
        << "内容";
}

SystemLogTableModel::~SystemLogTableModel() {}

int SystemLogTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mSystemLogDatas.size(); }

int SystemLogTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant SystemLogTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();
    if (row < 0 || row >= mSystemLogDatas.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const auto& data = mSystemLogDatas.at(row);
        switch (col)
        {
            //        case Id: return data.id;
        case Time:
        {
            return QDateTime::fromString(data.createTime, DATETIME_ORIGIN_FORMAT).toString(DATETIME_DISPLAY_FORMAT);
        }
        case Level: return SystemLogHelper::logLevelToString(data.level);
        case Type: return SystemLogHelper::logTypeString(data.type);
        case Module: return data.module;
        case UserId: return data.userID;
        case Context: return data.context.toUtf8();
        }
    }
    break;
    case Qt::TextAlignmentRole:
    {
        switch (col)
        {
        case Context: return Qt::AlignLeft;
        default: return Qt::AlignCenter;
        }
    }
    break;
    }

    return QVariant();
}

QVariant SystemLogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags SystemLogTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void SystemLogTableModel::setSystemLogData(const QList<SystemLogData>& systemLogDatas)
{
    beginResetModel();
    mSystemLogDatas = systemLogDatas;
    endResetModel();
}

QList<SystemLogData> SystemLogTableModel::getSystemLogData() const { return mSystemLogDatas; }

void SystemLogTableModel::appendSystemLogData(const SystemLogData& systemLogData)
{
    beginInsertRows(QModelIndex(), mSystemLogDatas.size(), mSystemLogDatas.size());
    mSystemLogDatas << systemLogData;
    endInsertRows();
}
