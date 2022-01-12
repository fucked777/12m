#include "TaskPlanTableModel.h"

TaskPlanTableModel::TaskPlanTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    mHeaders << "任务代号"
             << "系统工作模式"
             << "任务标识"
             << "系统工作方式"
             << "设备组合号"
             << "终端重组设备类型"
             << "任务开始时间"
             << "任务结束时间"
             << "航时"
             << "任务状态";
}

TaskPlanTableModel::~TaskPlanTableModel() {}

int TaskPlanTableModel::rowCount(const QModelIndex& parent) const { return 0; }

int TaskPlanTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant TaskPlanTableModel::data(const QModelIndex& index, int role) const { return QVariant(); }

QVariant TaskPlanTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags TaskPlanTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
