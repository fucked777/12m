#include "DeviceStatusLogTableModel.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"
#include "SqlDeviceStatusManager.h"
#include "StatusPersistenceMessageSerialize.h"
#include <QMessageBox>

const int loadNumberPerMore = 100;

DeviceStatusLogTableModel::DeviceStatusLogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    qRegisterMetaType<QSet<int>>("QSet<int>");
    qRegisterMetaType<QSet<int>>("QSet<int>&");
    qRegisterMetaType<QSet<int>>("const QSet<int>&");

    qRegisterMetaType<QSet<QString>>("QSet<QString>");
    qRegisterMetaType<QSet<QString>>("QSet<QString>&");
    qRegisterMetaType<QSet<QString>>("const QSet<QString>&");

    mHeaders << tr("时间") << tr("设备") << tr("模式") << tr("单元") << tr("参数") << tr("参数值");

    status_query_ = new SqlDeviceStatusManager;
    status_query_->moveToThread(&status_thread_);
    connect(&status_thread_, &QThread::finished, status_query_, &QObject::deleteLater);
    qRegisterMetaType<DeviceStatusLogDataList>("DeviceStatusLogDataList");

    connect(this, &DeviceStatusLogTableModel::searchLog, status_query_, &SqlDeviceStatusManager::searchLog);
    connect(status_query_, &SqlDeviceStatusManager::searchLogAck, this, &DeviceStatusLogTableModel::searchLogAck);
    connect(this, &DeviceStatusLogTableModel::totalCount, status_query_, &SqlDeviceStatusManager::totalCount);
    connect(status_query_, &SqlDeviceStatusManager::totalCountAck, this, &DeviceStatusLogTableModel::totalCountAck);
    //    res = connect(this, &DeviceStatusLogTableModel::load, status_query_, &SqlDeviceStatusManager::load, Qt::QueuedConnection);
    //    res = connect(status_query_, &SqlDeviceStatusManager::moreStatus, this, &DeviceStatusLogTableModel::moreStatus, Qt::QueuedConnection);
    //    res = connect(this, &DeviceStatusLogTableModel::search, status_query_, &SqlDeviceStatusManager::search, Qt::QueuedConnection);

    status_thread_.start();
}

DeviceStatusLogTableModel::~DeviceStatusLogTableModel() {}

int DeviceStatusLogTableModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : status_list_.size(); }

int DeviceStatusLogTableModel::columnCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : mHeaders.size(); }

QVariant DeviceStatusLogTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto col = index.column();
    if (row < 0 || row > status_list_.size() || col < 0 || col > mHeaders.size())
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const auto& data = status_list_.at(row);
        auto extention = GlobalData::getExtension(data.deviceId.toInt(0,16));
        switch (col)
        {
        case Time:
        {
            return Utility::dateTimeToStr(data.createTime);
        }
        case Device: return extention.desc;
        case ModeId:
        {
            if (!extention.haveMode)
            {
                return "无模式";
            }
            else
            {
                if (extention.modeCtrlMap.contains(data.modeId))
                {
                    return extention.modeCtrlMap[data.modeId].desc;
                }
                else
                {
                    return "模式ID错误";
                }
            }
        }
        case Unit: return data.unitId;
        case Param: return data.param;
        case ParamValue:
        {
            int unitId = -1;
            if (extention.modeCtrlMap.contains(data.modeId))
            {
                ModeCtrl modeCtrl = extention.modeCtrlMap[data.modeId];
                for (auto unit : modeCtrl.unitMap.values())
                {
                    if (unit.desc == data.unitId)
                    {
                        unitId = unit.unitCode;
                        break;
                    }
                }
            }

            if (unitId == -1)
            {
                return data.paramValue;
            }
            else
            {
                //                struct Unit unit = GlobalData::getUnit(deviceId, modeId, unitId);

                //                for (auto attr : unit.queryResultList)
                //                {
                //                    if (attr.desc == data.param)
                //                    {
                //                        if (unit.enumMap.contains(attr.id))
                //                        {
                //                            DevEnum devEnum = unit.enumMap[attr.id];
                //                            for (auto& entry : devEnum.emumEntryList)
                //                            {
                //                                if (data.paramValue == entry.uValue)
                //                                {
                //                                    paramValue = entry.desc;
                //                                    if (paramValue.contains(":"))
                //                                    {
                //                                        paramValue = paramValue.split(":").at(0);
                //                                    }
                //                                    break;
                //                                }
                //                            }
                //                        }
                //                        else
                //                        {
                //                            paramValue = data.paramValue;
                //                        }

                //                        if (!attr.unit.isEmpty())
                //                        {
                //                            paramValue += QString(" (%1)").arg(attr.unit);
                //                        }
                //                        break;
                //                    }
                //                }

                //                if (paramValue.isEmpty())
                //                {
                //                    paramValue = data.paramValue;
                //                }
                //                return paramValue;

                return data.paramValue;
            }
        }
        }
    }
    break;
    }

    return QVariant();
}

QVariant DeviceStatusLogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return mHeaders.at(section);
        }
    }
    default: return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags DeviceStatusLogTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractTableModel::flags(index);
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flag;
}

void DeviceStatusLogTableModel::setDeviceStatusLogData(const QList<DeviceStatusLogData>& deviceStatusLogDatas)
{
    beginResetModel();
    status_list_ = deviceStatusLogDatas;
    endResetModel();
}

QList<DeviceStatusLogData> DeviceStatusLogTableModel::getDeviceStatusLogData() const { return status_list_; }

void DeviceStatusLogTableModel::appendDeviceStatusLogData(const DeviceStatusLogData& deviceStatusLogData)
{
    beginInsertRows(QModelIndex(), status_list_.size(), status_list_.size());
    status_list_ << deviceStatusLogData;
    endInsertRows();
}

void DeviceStatusLogTableModel::reset(const QSet<int>& devices, const QSet<QString>& units, const QDateTime& start_time, const QDateTime& end_time,
               const QSet<int>& modes , int currentPage , int pageSize)
{
    if (!status_query_->queryFinished())
    {
        QMessageBox::information(nullptr, "提示", "上次状态数据还未查询完成", "确定");
        return;
    }
    emit searchLog(devices, units, start_time, end_time, modes, currentPage, pageSize);
    emit totalCount(devices, units, start_time, end_time, modes, currentPage);

    beginResetModel();
    status_list_.clear();
    endResetModel();
}

// void DeviceStatusLogTableModel::resetLog(const QString device, const QString deviceType, const QDateTime& start_time, const QDateTime& end_time)
//{
//    emit logsearch(device, deviceType, start_time, end_time);

//    beginResetModel();
//    status_list_.clear();
//    endResetModel();
//}

// void DeviceStatusLogTableModel::moreStatus(const DeviceStatusLogDataList& status)
//{
//    if (!status.isEmpty())
//    {
//        beginInsertRows(QModelIndex(), status_list_.count(), status_list_.count() + status.count() - 1);
//        status_list_ += status;
//        endInsertRows();
//        emit loaded(status.count());
//    }
//}

void DeviceStatusLogTableModel::searchLogAck(const DeviceStatusLogDataList& status)
{
    for (auto& s : status)
    {
        appendDeviceStatusLogData(s);
    }
}

void DeviceStatusLogTableModel::totalCountAck(const int totalCount, const int currentPage) { emit signalTotalCount(totalCount, currentPage); }

bool DeviceStatusLogTableModel::canFetchMore(const QModelIndex& /*parent*/) const { return /*status_query_->hasRecord()*/ false; }

void DeviceStatusLogTableModel::fetchMore(const QModelIndex& /*parent*/) { emit load(loadNumberPerMore); }
