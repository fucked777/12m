#include "StationResManagementModel.h"
#include "SRMUtility.h"
#include "StationResManagementSerialize.h"
#include <QDebug>
#include <tuple>

using ModelDataList = QList<std::tuple<StationResManagementData, QStringList>>;
class StationResManagementModelImpl
{
public:
    QStringList headList{ SRMUtility::modeHeader() };
    ModelDataList dataList;
};

StationResManagementModel::StationResManagementModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new StationResManagementModelImpl)
{
}
StationResManagementModel::~StationResManagementModel() { delete m_impl; }
void StationResManagementModel::addItem(const StationResManagementData& value)
{
    auto varList = SRMUtility::data2Mode(value);
    if (varList.size() != m_impl->headList.size())
    {
        /* 这个算BUG wp?? */
        qWarning() << "站资源数据和列表头数量不对应";
        return;
    }
    beginResetModel();
    m_impl->dataList << std::make_tuple(value, varList);
    endResetModel();
}
void StationResManagementModel::updateItem(const StationResManagementData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (SRMUtility::dataCompare(std::get<0>(item), value))
        {
            beginResetModel();
            item = std::make_tuple(value, SRMUtility::data2Mode(value));
            endResetModel();
            return;
        }
    }
}
void StationResManagementModel::deleteItem(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    auto row = index.row();
    if (row < 0 || row >= rowCount(index))
    {
        return;
    }
    beginResetModel();
    m_impl->dataList.removeAt(row);
    endResetModel();
}
void StationResManagementModel::deleteItem(const StationResManagementData& value)
{
    for (auto iter = m_impl->dataList.begin(); iter != m_impl->dataList.end(); ++iter)
    {
        if (SRMUtility::dataCompare(std::get<0>(*iter), value))
        {
            beginResetModel();
            m_impl->dataList.erase(iter);
            endResetModel();
            return;
        }
    }
}
void StationResManagementModel::updateData(const StationResManagementList& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, SRMUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void StationResManagementModel::updateData(const StationResManagementMap& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, SRMUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void StationResManagementModel::updateData()
{
    beginResetModel();
    endResetModel();
}
bool StationResManagementModel::dataExist(const StationResManagementData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (SRMUtility::dataCompare(std::get<0>(item), value))
        {
            return true;
        }
    }
    return false;
}
Optional<StationResManagementData> StationResManagementModel::getDataCenterItem(const QModelIndex& index)
{
    using ResType = Optional<StationResManagementData>;
    if (!index.isValid())
        return ResType(ErrorCode::InvalidArgument, "无效的索引");

    int row = index.row();  // 行号
    if (row < 0 || row >= rowCount(index))
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    return ResType(std::get<0>(m_impl->dataList.at(row)));
}
int StationResManagementModel::rowCount(const QModelIndex& /*parent*/) const { return m_impl->dataList.size(); }
int StationResManagementModel::columnCount(const QModelIndex& /*parent*/) const { return m_impl->headList.size(); }

QVariant StationResManagementModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();  // 行号

    if (row < 0 || row >= rowCount(index))
        return QVariant();

    int colnum = index.column();
    auto& data = m_impl->dataList.at(row);
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return std::get<1>(data).at(colnum);
        break;
    }
    case Qt::CheckStateRole: return QVariant();
    }

    return QVariant();
}

QVariant StationResManagementModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_impl->headList.at(section);
        }
    }
    default: return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags StationResManagementModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
bool StationResManagementModel::judgeData(int row) { return (row >= 0 && row < rowCount()); }
