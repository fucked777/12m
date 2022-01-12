
#include "AutomateTestResultModel.h"
#include "AutomateTestDefine.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

AutomateTestResultModel::AutomateTestResultModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "测试计划名称"
               << "测试项名称"
               << "测试日期";
}

void AutomateTestResultModel::updateData(const AutomateTestResultList& logs)
{
    beginResetModel();
    records = logs;
    endResetModel();
}

void AutomateTestResultModel::clear()
{
    beginResetModel();
    records.clear();
    endResetModel();
}
void AutomateTestResultModel::removeValue(const AutomateTestResult& removeItem)
{
    beginResetModel();
    for (auto iter = records.begin(); iter != records.end(); ++iter)
    {
        if (iter->id == removeItem.id)
        {
            records.erase(iter);
            break;
        }
    }
    endResetModel();
}
Optional<AutomateTestResult> AutomateTestResultModel::getItem(const QModelIndex& index) const
{
    using ResType = Optional<AutomateTestResult>;
    auto row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "当前索引越界");
    }

    return ResType(records.at(row));
}
QVariant AutomateTestResultModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int AutomateTestResultModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int AutomateTestResultModel::columnCount(const QModelIndex& /* parent */) const { return headerList.size(); }

QVariant AutomateTestResultModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row < 0 || row >= rowCount())
    {
        return QVariant();
    }

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
            return record.testPlanName;
        }
        case 1:
        {
            return record.testTypeName;
        }
        case 2:
        {
            return Utility::dateTimeToStr(record.testTime);
        }
        }
    }
    }
    return QVariant();
}

Qt::ItemFlags AutomateTestResultModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
