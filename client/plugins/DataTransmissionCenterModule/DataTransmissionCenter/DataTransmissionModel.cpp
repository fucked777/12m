#include "DataTransmissionModel.h"
#include "DTCUtility.h"
#include "DataTransmissionCenterSerialize.h"
#include <QDebug>
#include <tuple>

using ModelDataList = QList<std::tuple<DataTransmissionCenterData, QStringList>>;
class DataTransmissionModelImpl
{
public:
    QStringList headList{ DTCUtility::modeHeader() };
    ModelDataList dataList;
};

DataTransmissionModel::DataTransmissionModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new DataTransmissionModelImpl)
{
}
DataTransmissionModel::~DataTransmissionModel() { delete m_impl; }
void DataTransmissionModel::addItem(const DataTransmissionCenterData& value)
{
    auto varList = DTCUtility::data2Mode(value);
    if (varList.size() != m_impl->headList.size())
    {
        /* 这个算BUG wp?? */
        qWarning() << "数传中心数据和列表头数量不对应";
        return;
    }
    beginResetModel();
    m_impl->dataList << std::make_tuple(value, varList);
    endResetModel();
}
void DataTransmissionModel::updateItem(const DataTransmissionCenterData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (DTCUtility::dataCompare(std::get<0>(item), value))
        {
            beginResetModel();
            item = std::make_tuple(value, DTCUtility::data2Mode(value));
            endResetModel();
            return;
        }
    }
}
void DataTransmissionModel::deleteItem(const QModelIndex& index)
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
void DataTransmissionModel::deleteItem(const DataTransmissionCenterData& value)
{
    for (auto iter = m_impl->dataList.begin(); iter != m_impl->dataList.end(); ++iter)
    {
        if (DTCUtility::dataCompare(std::get<0>(*iter), value))
        {
            beginResetModel();
            m_impl->dataList.erase(iter);
            endResetModel();
            return;
        }
    }
}
void DataTransmissionModel::updateData(const DataTransmissionCenterList& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, DTCUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void DataTransmissionModel::updateData(const DataTransmissionCenterMap& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, DTCUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void DataTransmissionModel::updateData()
{
    beginResetModel();
    endResetModel();
}
bool DataTransmissionModel::dataExist(const DataTransmissionCenterData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (DTCUtility::dataCompare(std::get<0>(item), value))
        {
            return true;
        }
    }
    return false;
}
Optional<DataTransmissionCenterData> DataTransmissionModel::getDataCenterItem(const QModelIndex& index)
{
    using ResType = Optional<DataTransmissionCenterData>;
    if (!index.isValid())
        return ResType(ErrorCode::InvalidArgument, "无效的索引");

    int row = index.row();  // 行号
    if (row < 0 || row >= rowCount(index))
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    return ResType(std::get<0>(m_impl->dataList.at(row)));
}
int DataTransmissionModel::rowCount(const QModelIndex& /*parent*/) const { return m_impl->dataList.size(); }
int DataTransmissionModel::columnCount(const QModelIndex& /*parent*/) const { return m_impl->headList.size(); }

QVariant DataTransmissionModel::data(const QModelIndex& index, int role) const
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
    }
    case Qt::CheckStateRole: return QVariant();
    }

    return QVariant();
}

QVariant DataTransmissionModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags DataTransmissionModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
bool DataTransmissionModel::judgeData(int row) { return (row >= 0 && row < rowCount()); }
