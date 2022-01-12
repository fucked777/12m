
#include "AutomateTestModel.h"
#include "AutomateTestDefine.h"
#include <QJsonDocument>
#include <QJsonObject>

AutomateTestModel::AutomateTestModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "测试计划名称"
               << "工作模式"
               << "测试类型"
               << "测试分系统";
}

void AutomateTestModel::setTestItemData(const QList<AutomateTestPlanItem>& tempList)
{
    QList<AutomateTestModelInfo> infoList;
    for (auto& item : tempList)
    {
        AutomateTestModelInfo tempInfo;
        tempInfo.automateTestPlanItem = item;
        tempInfo.groupName = item.groupName;
        tempInfo.testName = item.testName;
        tempInfo.testTypeName = item.testTypeName;
        tempInfo.workMode = SystemWorkModeHelper::systemWorkModeToDesc(item.workMode);
        infoList << tempInfo;
    }
    beginResetModel();
    records = infoList;
    endResetModel();
}
void AutomateTestModel::clear()
{
    beginResetModel();
    records.clear();
    endResetModel();
}
bool AutomateTestModel::isValid() { return !records.isEmpty(); }

QVariant AutomateTestModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int AutomateTestModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int AutomateTestModel::columnCount(const QModelIndex& /* parent */) const { return headerList.count(); }

QVariant AutomateTestModel::data(const QModelIndex& index, int role) const
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
            return record.testName;
        }
        case 1:
        {
            return record.workMode;
        }
        case 2:
        {
            return record.testTypeName;
        }
        case 3:
        {
            return record.groupName;
        }
        }
    }
    }
    return QVariant();
}

Qt::ItemFlags AutomateTestModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
bool AutomateTestModel::testItem(const QModelIndex& index, AutomateTestPlanItem& item)
{
    auto row = index.row();
    if (row >= 0 && row < rowCount())
    {
        item = records.at(row).automateTestPlanItem;
        return true;
    }
    return false;
}
