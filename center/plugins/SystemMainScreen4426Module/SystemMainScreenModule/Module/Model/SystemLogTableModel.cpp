#include "SystemLogTableModel.h"

#include "SystemLogMessageDefine.h"
#include "SystemLogMessageSerialize.h"

#include <QColor>
#include <QDateTime>

SystemLogTableModel::SystemLogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "类型"
             << "时间"
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
    if (row < 0 || row > mSystemLogDatas.size() || col < 0 || col > mHeaders.size())
    {
        return QVariant();
    }

    const auto& data = mSystemLogDatas.at(row);
    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case Type: return SystemLogHelper::logLevelToString(data.level);
        case Time: return QDateTime::fromString(data.createTime, DATETIME_ORIGIN_FORMAT).toString(DATETIME_DISPLAY_FORMAT);
        case Context: return data.context;
        }
    }
    break;
    case Qt::TextAlignmentRole:
    {
        switch (col)
        {
        case Type:
        case Time: return QVariant(Qt::AlignCenter);
        case Context: return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    case Qt::ForegroundRole:
    {
        switch (col)
        {
        case Type:
        case Time:
        case Context:
            switch (data.level)
            {
            case LogLevel::Warning: return QColor(229, 153, 0);
            case LogLevel::Error: return QColor(231, 35, 35);
            case LogLevel::SpecificTips: return QColor(0, 170, 0);
            }
        }
        return QVariant();
    }
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

void SystemLogTableModel::prependSystemLogData(const SystemLogData& systemLogData)
{
    beginResetModel();
    if (mSystemLogDatas.size() > 400)
    {
        mSystemLogDatas.removeLast();
    }

    mSystemLogDatas.prepend(systemLogData);
    endResetModel();
}

void SystemLogTableModel::clear()
{
    beginResetModel();
    mSystemLogDatas.clear();
    endResetModel();
}

QList<SystemLogData> SystemLogTableModel::getCurrentLogData() { return mSystemLogDatas; }
