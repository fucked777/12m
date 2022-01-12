#include "ParameterBindingModel.h"
#include "AutomateTestDefine.h"
#include "AutomateTestSerialize.h"
#include "AutomateTestUtility.h"
#include "GlobalData.h"
#include "SystemWorkMode.h"
#include "ui_ParameterBindingConfig.h"
#include <QComboBox>
#include <QDateTime>
#include <QLabel>

ParameterBindingModel::ParameterBindingModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}
void ParameterBindingModel::setIndexData(const ParameterBindingConfigInfo& info)
{
    beginResetModel();
    records.clear();

    bindingConfigInfo = info;
    headList.clear();
    headList.push_back("序号");
    for (auto& item : bindingConfigInfo.headList)
    {
        headList.push_back(std::get<1>(item));
    }

    endResetModel();
}
void ParameterBindingModel::updateData(const AutomateTestParameterBindGroup& group) { updateData(group.bindDataList); }
void ParameterBindingModel::updateData(const QList<AutomateTestParameterBind>& bindDataList)
{
    QList<ParameterBindingModelDataItem> tempList;
    int i = 1;
    for (auto& groupitem : bindDataList)
    {
        ParameterBindingModelDataItem tempDataItem;
        tempDataItem.bind = groupitem;
        tempDataItem.varList << QString::number(i++);
        for (auto& item : bindingConfigInfo.headList)
        {
            auto value = groupitem.testBindParameterMap.value(std::get<0>(item));
            if (value.displayValue.isEmpty())
            {
                tempDataItem.varList << "-";
            }
            else
            {
                tempDataItem.varList << value.displayValue;
            }
        }

        tempList << tempDataItem;
    }
    beginResetModel();
    records = tempList;
    endResetModel();
}
void ParameterBindingModel::updateData(const AutomateTestParameterBind& bindItem)
{
    ParameterBindingModelDataItem tempDataItem;
    tempDataItem.bind = bindItem;
    tempDataItem.varList << QString::number(1);
    for (auto& item : bindingConfigInfo.headList)
    {
        auto value = bindItem.testBindParameterMap.value(std::get<0>(item));
        if (value.displayValue.isEmpty())
        {
            tempDataItem.varList << "-";
        }
        else
        {
            tempDataItem.varList << value.displayValue;
        }
    }
    beginResetModel();
    records.clear();
    records << tempDataItem;
    endResetModel();
}
QVariant ParameterBindingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return headList.at(section);
        }
    }
    }
    return QVariant();
}

int ParameterBindingModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int ParameterBindingModel::columnCount(const QModelIndex& /* parent */) const { return headList.size(); }

QVariant ParameterBindingModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row < 0 || row >= rowCount() || col < 0 || col >= columnCount())
    {
        return QVariant();
    }
    auto& record = records.at(row);

    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return record.varList.at(col);
    }
    }
    return QVariant();
}

Qt::ItemFlags ParameterBindingModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
bool ParameterBindingModel::itemAt(AutomateTestParameterBind& item, const QModelIndex& index) const
{
    auto row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return false;
    }
    item = records.at(row).bind;
    return true;
}
