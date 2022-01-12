#include "EphemerisDataModel.h"
#include "EDUtility.h"
#include "EphemerisDataSerialize.h"
#include <QDebug>
#include <tuple>

using ModelDataList = QList<std::tuple<EphemerisDataData, QStringList>>;
class EphemerisDataModelImpl
{
public:
    QStringList headList{ EDUtility::modeHeader() };
    ModelDataList dataList;
};

EphemerisDataModel::EphemerisDataModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new EphemerisDataModelImpl)
{
}
EphemerisDataModel::~EphemerisDataModel() { delete m_impl; }
void EphemerisDataModel::addItem(const EphemerisDataData& value)
{
    auto varList = EDUtility::data2Mode(value);
    if (varList.size() != m_impl->headList.size())
    {
        /* 这个算BUG wp?? */
        qWarning() << "星历数据数据和列表头数量不对应";
        return;
    }
    beginResetModel();
    m_impl->dataList << std::make_tuple(value, varList);
    endResetModel();
}
void EphemerisDataModel::updateItem(const EphemerisDataData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (EDUtility::dataCompare(std::get<0>(item), value))
        {
            beginResetModel();
            item = std::make_tuple(value, EDUtility::data2Mode(value));
            endResetModel();
            return;
        }
    }
}
void EphemerisDataModel::deleteItem(const QModelIndex& index)
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
void EphemerisDataModel::deleteItem(const EphemerisDataData& value)
{
    for (auto iter = m_impl->dataList.begin(); iter != m_impl->dataList.end(); ++iter)
    {
        if (EDUtility::dataCompare(std::get<0>(*iter), value))
        {
            beginResetModel();
            m_impl->dataList.erase(iter);
            endResetModel();
            return;
        }
    }
}
void EphemerisDataModel::updateData(const EphemerisDataList& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, EDUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void EphemerisDataModel::updateData(const EphemerisDataMap& logs)
{
    ModelDataList temp;
    for (auto& item : logs)
    {
        temp << std::make_tuple(item, EDUtility::data2Mode(item));
    }

    beginResetModel();
    m_impl->dataList = temp;
    endResetModel();
}
void EphemerisDataModel::updateData()
{
    beginResetModel();
    endResetModel();
}
bool EphemerisDataModel::dataExist(const EphemerisDataData& value)
{
    for (auto& item : m_impl->dataList)
    {
        if (EDUtility::dataCompare(std::get<0>(item), value))
        {
            return true;
        }
    }
    return false;
}
Optional<EphemerisDataData> EphemerisDataModel::getItem(const QModelIndex& index)
{
    using ResType = Optional<EphemerisDataData>;
    if (!index.isValid())
        return ResType(ErrorCode::InvalidArgument, "无效的索引");

    int row = index.row();  // 行号
    if (row < 0 || row >= rowCount(index))
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    return ResType(std::get<0>(m_impl->dataList.at(row)));
}
int EphemerisDataModel::rowCount(const QModelIndex& /*parent*/) const { return m_impl->dataList.size(); }
int EphemerisDataModel::columnCount(const QModelIndex& /*parent*/) const { return m_impl->headList.size(); }

QVariant EphemerisDataModel::data(const QModelIndex& index, int role) const
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

QVariant EphemerisDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags EphemerisDataModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}
bool EphemerisDataModel::judgeData(int row) { return (row >= 0 && row < rowCount()); }
