#include "SatelliteTableViewModel.h"
#include "ExtendedConfig.h"
#include "SatelliteManagementDefine.h"
SatelliteTableViewModel::SatelliteTableViewModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_currentProject(ExtendedConfig::curProjectID())
{
}

SatelliteTableViewModel::~SatelliteTableViewModel() {}

void SatelliteTableViewModel::setWorkModeMap(const QMap<int, QString>& workModeMap) { m_workModeMap = workModeMap; }

int SatelliteTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int SatelliteTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_HorizontalHeader.count();
}

QVariant SatelliteTableViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }
    if (role == Qt::DisplayRole)
    {
        int ncol = index.column();
        int nrow = index.row();
        auto item = m_data.at(nrow);

        switch (ncol)
        {
        case 0: return item.m_satelliteCode;
        case 1: return item.m_satelliteIdentification;
        case 2: return item.m_satelliteName;
        case 3: return item.m_taskCenterID;
        case 4: return item.m_digitalCenterID;
        case 5: return item.railTypeString();
        case 6: return item.trackingModeString(m_currentProject);
        case 7:
        {
            QStringList workMode;
            auto workList = item.m_workMode.split("+");
            for (auto workItem : workList)
            {
                workMode.append(m_workModeMap.value(workItem.toInt()));
            }
            return workMode.join("+");
        }

        default: return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags SatelliteTableViewModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    // flag|=Qt::ItemIsEditable // 设置单元格可编辑,此处注释,单元格无法被编辑
    return flag;
}

void SatelliteTableViewModel::setHorizontalHeader(const QStringList& headers) { m_HorizontalHeader = headers; }

QVariant SatelliteTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return m_HorizontalHeader.at(section);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void SatelliteTableViewModel::setSatelliteData(const QList<SatelliteManagementData>& data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

QList<SatelliteManagementData>& SatelliteTableViewModel::DataVector() { return m_data; }

QString SatelliteTableViewModel::satelliteID(const QModelIndex index)
{
    if (index.row() < m_data.size())
    {
        return m_data[index.row()].m_satelliteCode;
    }
    return "";
}

SatelliteManagementData SatelliteTableViewModel::SatelliteData(const QModelIndex index)
{
    if (index.row() < m_data.size())
    {
        return m_data[index.row()];
    }
    return SatelliteManagementData();
}
