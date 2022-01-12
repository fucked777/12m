#include "TableModel.h"
#include "MircoServiceInfo.h"
TableModel::TableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_HorizontalHeader << "ID"
                       << "名称"
                       << "符号"
                       << "版本"
                       << "作者"
                       << "描述"
                       << "更新日志"
                       << "操作";
}

TableModel::~TableModel() {}

MircoServiceInfo TableModel::rowData(int row)
{
    if (row < m_data.size())
    {
        return m_data[row];
    }
    return MircoServiceInfo();
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return NUM + 1;
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    {
        int ncol = index.column();
        int nrow = index.row();
        auto item = m_data.at(nrow);
        switch (ncol)
        {
        case SERVICE_ID: return item.m_serviceID; break;
        case SERVICE_NAME: return item.m_serviceName; break;
        case SYMBOLIC_NAME: return item.m_symbolicName; break;
        case SERVICE_VERSION: return item.m_serviceVersion; break;
        case SERVICE_AUTHORS: return item.m_serviceAuthors; break;
        case SERVICE_DESCRIPTION: return item.m_serviceDescription; break;
        case SERVCIE_UPDATE_LOG: return item.m_serviceLog; break;
        default: return QVariant(); break;
        }
    }
    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    // flag|=Qt::ItemIsEditable // 设置单元格可编辑,此处注释,单元格无法被编辑
    return flag;
}

void TableModel::setHorizontalHeader(const QStringList& headers) { m_HorizontalHeader = headers; }

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return m_HorizontalHeader.at(section);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void TableModel::setData(const QList<MircoServiceInfo>& microServiceInfoList)
{
    m_data = microServiceInfoList;
    beginResetModel();
    endResetModel();
}
