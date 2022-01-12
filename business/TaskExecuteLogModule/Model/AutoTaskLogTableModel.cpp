#include "AutoTaskLogTableModel.h"

#include "AutoRunTaskLogMessageDefine.h"

#include <QDateTime>
AutoTaskLogTableModel::AutoTaskLogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "时间"
             << "任务代号"
             << "级别"
             << "内容";
}

AutoTaskLogTableModel::~AutoTaskLogTableModel() {}

int AutoTaskLogTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mAutoTaskLogs.size(); }

int AutoTaskLogTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant AutoTaskLogTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();

    if (row < 0 || row >= mAutoTaskLogs.size() || col < 0 || col >= mHeaders.size())
    {
        return QVariant();
    }

    auto& data = mAutoTaskLogs.at(row);
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case CreateTime: return QDateTime::fromString(data.createTime, DATETIME_ORIGIN_FORMAT).toString(DATETIME_DISPLAY_FORMAT);
        case TaskCode: return data.taskCode;
        case Level: return AutoTaskLogHelper::logLevelToString(data.level);
        case Context: return data.context;
        }
    }
    break;
    case Qt::TextColorRole:
    {
        return AutoTaskLogHelper::logLevelToColor(data.level);
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

QVariant AutoTaskLogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags AutoTaskLogTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void AutoTaskLogTableModel::setAutoTaskLogDatas(const QList<AutoTaskLogData>& list)
{
    beginResetModel();
    mAutoTaskLogs = list;
    endResetModel();
}

Optional<AutoTaskLogData> AutoTaskLogTableModel::getAutoTaskLogData(const QModelIndex& index) const
{
    using ResType = Optional<AutoTaskLogData>;
    if (!index.isValid())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(mAutoTaskLogs.at(row));
}
