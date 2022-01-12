
#include "HistryDataModel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

HistryDataModel::HistryDataModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "序号"
               << "测试时间"
               << "任务代号"
               << "点频号";
    //<< "下行频率(MHz)";

    // 测试数据
    //    AGCCalibrationItemList temptemp;
    //    for (int i = 0; i < 10; ++i)
    //    {
    //        AGCCalibrationItem tempItem;

    //        tempItem.itemGUID = QUuid::createUuid().toString();
    //        tempItem.taskCode = "708l";
    //        tempItem.createTime = QDateTime::currentDateTime();
    //        tempItem.projectCode = "442x";
    //        tempItem.taskID = "wx";
    //        tempItem.workMode = SystemWorkMode::STTC;
    //        tempItem.downFreq = 19200;
    //        tempItem.dotDrequencyNum = 1;

    //        for (int j = 0; j < 10; ++j)
    //        {
    //            AGCResult tempAGCResult;
    //            tempAGCResult.agc = i * j;
    //            tempAGCResult.sbf = tempAGCResult.agc * j;
    //            tempItem.result << tempAGCResult;
    //        }
    //        temptemp << tempItem;
    //    }
    //    updateData(temptemp);
}

void HistryDataModel::updateData()
{
    beginResetModel();
    endResetModel();
}

void HistryDataModel::updateData(const AGCCalibrationItemList& list)
{
    beginResetModel();
    records = list;
    endResetModel();
}
void HistryDataModel::appendData(const AGCCalibrationItemList& list)
{
    beginResetModel();
    records << list;
    endResetModel();
}
void HistryDataModel::removeValue(const AGCCalibrationItem& removeItem)
{
    beginResetModel();
    for (auto iter = records.begin(); iter != records.end(); ++iter)
    {
        if (iter->itemGUID == removeItem.itemGUID)
        {
            records.erase(iter);
            break;
        }
    }
    endResetModel();
}
QVariant HistryDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int HistryDataModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int HistryDataModel::columnCount(const QModelIndex& /* parent */) const { return headerList.size(); }
void HistryDataModel::clear()
{
    beginResetModel();
    records.clear();
    endResetModel();
}
QVariant HistryDataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    auto& record = records.at(row);

    switch (role)
    {
    case Qt::TextAlignmentRole:
    {
        return QVariant(Qt::AlignCenter);
    }
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case 0:
        {
            return row + 1;
        }
        case 1:
        {
            return record.createTime.toString("yyyy-MM-dd hh:mm:ss");
        }
        case 2:
        {
            return record.taskCode;
        }
        case 3:
        {
            return record.dotDrequencyNum;
            // return QString::number(record.downFreq, 'f');
        }
        }
    }
    }
    return QVariant();
}
bool HistryDataModel::agcItemData(const QModelIndex& index, AGCCalibrationItem& item)
{
    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return false;
    }
    item = records.at(row);
    return true;
}
Qt::ItemFlags HistryDataModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
