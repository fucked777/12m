
#include "AGCAndSBFModel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

AGCAndSBFModel::AGCAndSBFModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_flag(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable)
{
    m_headerList << "序号"
                 << "载噪比"
                 << "AGC电压(V)";

    // 测试数据
    //    QList<AGCResult> values;
    //    for (int i = 0; i < 10; ++i)
    //    {
    //        AGCResult tempData;
    //        tempData.agc = 1.0 + i;
    //        tempData.sbf = 10.0 + i;
    //        values << tempData;
    //    }
    //    m_records.itemGUID = "xxxxxxxxxxxx";
    //    updateData(values);
}

void AGCAndSBFModel::updateData()
{
    beginResetModel();
    endResetModel();
}

void AGCAndSBFModel::addItem(const AGCResult& info)
{
    beginResetModel();
    m_records.result << info;
    endResetModel();
}
bool AGCAndSBFModel::removeItem(const QModelIndex& index)
{
    if (judgeData(index))
    {
        beginResetModel();
        m_records.result.removeAt(index.row());
        endResetModel();
        return true;
    }
    return false;
}
void AGCAndSBFModel::addItem(const QModelIndex& index)
{
    /* 如果row越界就在最后添加,否则在指定的行后添加 */
    auto insertRow = rowCount();
    if (judgeData(index))
    {
        insertRow = index.row();
    }

    beginResetModel();
    m_records.result.insert(insertRow, AGCResult());
    endResetModel();
}
void AGCAndSBFModel::updateData(const QList<AGCResult>& values)
{
    beginResetModel();
    m_records.result = values;
    endResetModel();
}

QVariant AGCAndSBFModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_headerList.at(section);
        }
    }
    }
    return QVariant();
}
/* 多一行用于编辑 */
int AGCAndSBFModel::rowCount(const QModelIndex& /* parent */) const { return m_records.result.count(); }
int AGCAndSBFModel::columnCount(const QModelIndex& /* parent */) const { return m_headerList.size(); }

QVariant AGCAndSBFModel::data(const QModelIndex& index, int role) const
{
    if (!judgeData(index))
        return QVariant();

    int row = index.row();
    int col = index.column();

    auto& record = m_records.result.at(row);

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
            return record.sbf;
        }
        case 2:
        {
            return record.agc;
        }
        }
    }
    }
    return QVariant();
}

bool AGCAndSBFModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!judgeData(index))
        return false;

    //处理 Qt::EditRole 角色，此时的 value 就是修改后的值
    //这里我是数字但是这里传入的QVariant类型是QString 字符串为空是很正常的
    //这里判断一下字符串为空就认为无效
    if (role != Qt::EditRole || value.toString().isEmpty())
    {
        return false;
    }

    int row = index.row();
    int col = index.column();
    auto& record = m_records.result[row];

    switch (col)
    {
    case 0:
    {
        return false;
    }
    case 1:
    {
        record.sbf = value.toDouble();
        break;
    }
    case 2:
    {
        record.agc = value.toDouble();
        break;
    }
    }

    emit sg_dataChange();
    m_isChangeValue = true;
    return true;
}
Qt::ItemFlags AGCAndSBFModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    /* 第0列是序号禁止编辑 */
    if (index.column() == 0)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return m_flag;
}
void AGCAndSBFModel::setItemValue(const AGCCalibrationItem& temp)
{
    beginResetModel();
    m_records = temp;
    endResetModel();
    emit sg_dataIsValid(isValid());
}
void AGCAndSBFModel::resetItemValue()
{
    beginResetModel();
    m_records = AGCCalibrationItem();
    m_isChangeValue = false;
    endResetModel();
    emit sg_dataIsValid(isValid());
}
bool AGCAndSBFModel::isChangeValue() { return m_isChangeValue; }
void AGCAndSBFModel::setEnableEdit(bool isEnable)
{
    m_flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (isEnable)
    {
        m_flag |= Qt::ItemIsEditable;
    }
}
bool AGCAndSBFModel::enableEdit() const { return (m_flag & Qt::ItemIsEditable) == Qt::ItemIsEditable; }
bool AGCAndSBFModel::judgeData(const QModelIndex& index) const
{
    auto row = index.row();
    return (row >= 0 && row < rowCount());
}
