#include "RealTimeLogModel.h"

#include <QDateTime>

RealtimeLogModel::RealtimeLogModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

QVariant RealtimeLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case Time: return QString("时间");
            case TaskId: return QString("计划编号");
            case Level:
                return QString("类型");
                //            case taskStep:
                //                return QString("生成插件");
            case Context: return QString("内容");
            default: return QVariant();
            }
        }
    }
    }
    return QVariant();
}

int RealtimeLogModel::rowCount(const QModelIndex& /* parent */) const { return mAutoTaskLogData.size(); }

int RealtimeLogModel::columnCount(const QModelIndex& /*parent*/) const { return Columnsize; }

QVariant RealtimeLogModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();  //行号
    if (row < 0 || row >= rowCount())
        return QVariant();

    int colnum = index.column();
    auto data = mAutoTaskLogData.at(row);
    switch (role)
    {
    case Qt::TextAlignmentRole:
    {
        if (colnum < Columnsize - 1)
        {
            return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        }
        else
        {
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    case Qt::DisplayRole:
    {
        switch (colnum)
        {
        case Time: return QDateTime::fromString(data.createTime, DATETIME_ORIGIN_FORMAT).toString(DATETIME_DISPLAY_FORMAT); ;
        case TaskId: return data.uuid;
        case Level: return AutoTaskLogHelper::logLevelToString(data.level);
        case Context: return data.context;
        }
    }
    case Qt::TextColorRole:
    {
        return AutoTaskLogHelper::logLevelToColor(data.level);
    }
    }
    return QVariant();
}

Qt::ItemFlags RealtimeLogModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void RealtimeLogModel::prependLogData(const AutoTaskLogData& log)
{
    beginResetModel();
    if (mAutoTaskLogData.size() > 400)
    {
        mAutoTaskLogData.removeLast();
    }

    mAutoTaskLogData.prepend(log);
    endResetModel();
}

bool RealtimeLogModel::judgeData(int row) { return (row >= 0 && row < rowCount()); }

void RealtimeLogModel::clear()
{
    beginResetModel();
    mAutoTaskLogData.clear();
    endResetModel();
}
