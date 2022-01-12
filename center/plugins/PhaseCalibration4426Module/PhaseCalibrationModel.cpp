#include "PhaseCalibrationModel.h"
#include "PhaseCalibrationSerialize.h"
#include "SystemWorkMode.h"
#include <QJsonDocument>
#include <QJsonObject>

PhaseCalibrationModel::PhaseCalibrationModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "校零时间"
               << "项目代号"
               << "任务代号"
               << "工作模式"
               << "下行频率"
               << "中心体温度"
               << "中心体湿度"
               << "方位移相值"
               << "俯仰移相值"
               << "方位斜率"
               << "俯仰斜率";
}

void PhaseCalibrationModel::setValue(const PhaseCalibrationItemList& listCI)
{
    beginResetModel();
    records = listCI;
    endResetModel();
}
void PhaseCalibrationModel::removeValue(const PhaseCalibrationItem& removeItem)
{
    for (auto iter = records.begin(); iter != records.end(); ++iter)
    {
        if (iter->id == removeItem.id)
        {
            beginResetModel();
            records.erase(iter);
            endResetModel();
            break;
        }
    }
}
void PhaseCalibrationModel::clear()
{
    beginResetModel();
    records.clear();
    endResetModel();
}

QVariant PhaseCalibrationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return headerList.at(section);
        }
    }
    }
    return QVariant();
}

int PhaseCalibrationModel::rowCount(const QModelIndex& /* parent */) const { return records.size(); }
int PhaseCalibrationModel::columnCount(const QModelIndex& /* parent */) const { return headerList.size(); }

QVariant PhaseCalibrationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    auto& record = records.at(row);

    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case 0:
        {
            return record.createTime;
        }
        case 1:
        {
            return record.projectCode;
        }
        case 2:
        {
            return record.taskCode;
        }
        case 3:
        {
            return SystemWorkModeHelper::systemWorkModeToDesc(record.workMode);
        }
        case 4:
        {
            return record.downFreq;
        }
        case 5:
        {
            return record.temperature;
        }
        case 6:
        {
            return record.humidity;
        }
        case 7:
        {
            return record.azpr;
        }
        case 8:
        {
            return record.elpr;
        }
        case 9:
        {
            return record.azge;
        }
        case 10:
        {
            return record.elge;
        }
        }
    }
    }
    return QVariant();
}

Qt::ItemFlags PhaseCalibrationModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
bool PhaseCalibrationModel::itemAt(const QModelIndex& index, PhaseCalibrationItem& item)
{
    auto row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return false;
    }

    item = records.at(row);
    return true;
}
