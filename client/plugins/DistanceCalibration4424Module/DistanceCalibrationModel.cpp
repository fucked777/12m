#include "DistanceCalibrationModel.h"
#include "DistanceCalibrationSerialize.h"
#include "SystemWorkMode.h"
#include <QJsonDocument>
#include <QJsonObject>

DistanceCalibrationModel::DistanceCalibrationModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "校零时间"
               << "项目代号"
               << "任务代号"
               << "工作模式"
               << "上行频率"
               << "下行频率"
               << "距离零值均值"
               << "距离零值方差";
}

void DistanceCalibrationModel::updateData()
{
    beginResetModel();
    endResetModel();
}
void DistanceCalibrationModel::setValue(const DistanceCalibrationItemList& listCI)
{
    beginResetModel();
    records = listCI;
    endResetModel();
}

void DistanceCalibrationModel::appendValue(const DistanceCalibrationItemList& listCI)
{
    beginResetModel();
    records << listCI;
    endResetModel();
}
void DistanceCalibrationModel::clear()
{
    beginResetModel();
    records.clear();
    endResetModel();
}

void DistanceCalibrationModel::removeValue(const DistanceCalibrationItem& removeItem)
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

QVariant DistanceCalibrationModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int DistanceCalibrationModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int DistanceCalibrationModel::columnCount(const QModelIndex& /* parent */) const { return headerList.size(); }

QVariant DistanceCalibrationModel::data(const QModelIndex& index, int role) const
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
            return record.taskIdent;
        }
        case 3:
        {
            switch (record.workMode)
            {
            case STTC:
            {
                return "S标准TTC";
            }
            case Skuo2:
            {
                return "S扩二";
            }
            case SYTHSMJ:
            {
                return "一体化上面级";
            }
            case SYTHWX:
            {
                return "一体化卫星";
            }
            case SYTHGML:
            {
                return "一体化高码率";
            }
            case SYTHSMJK2GZB:
            {
                return "一体化上面级+扩二共载波";
            }
            case SYTHSMJK2BGZB:
            {
                return "一体化上面级+扩二不共载波";
            }
            case SYTHWXSK2:
            {
                return "一体化卫星+扩二";
            }
            case SYTHGMLSK2:
            {
                return "一体化高码率+扩二";
            }
            case SKT:
            {
                return "扩跳";
            }
            }
        }
        case 4:
        {
            return record.uplinkFreq;
        }
        case 5:
        {
            return record.downFreq;
        }
        case 6:
        {
            return record.distZeroMean;
        }
        case 7:
        {
            return record.distZeroVar;
        }
        }
    }
    }
    return QVariant();
}

Qt::ItemFlags DistanceCalibrationModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
bool DistanceCalibrationModel::itemAt(DistanceCalibrationItem& item, const QModelIndex& index)
{
    auto row = index.row();
    if (row >= 0 && row < rowCount())
    {
        item = records.at(row);
        return true;
    }
    return false;
}
