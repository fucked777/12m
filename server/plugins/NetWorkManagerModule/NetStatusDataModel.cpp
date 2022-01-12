#include "NetStatusDataModel.h"
#include "CommunicationSerialize.h"
#include "NetStatusInterface.h"
#include <QJsonDocument>
#include <QJsonObject>

NetStatusDataModel::NetStatusDataModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    headerList << "通道ID"
               << "描述"
               << "通讯协议"
               << "本地IP"
               << "目标IP"
               << "接收端口"
               << "发送端口"
               << "发送包计数"
               << "发送包错误计数"
               << "发送链接状态"
               << "接收包计数"
               << "接收包错误计数"
               << "接收链接状态";
}

void NetStatusDataModel::updateData()
{
    beginResetModel();
    endResetModel();
}
QString convetCommunicationTypeToDesc(CommunicationType value)
{
    switch (value)
    {
    case CommunicationType::Unknow:
    {
        break;
    }
    case CommunicationType::UDPGB:
    {
        return "UDP组播";
    }
    case CommunicationType::UDPP2P:
    {
        return "UDP点对点";
    }
    case CommunicationType::TCPServer:
    {
        return "TCP服务端";
    }
    case CommunicationType::TCPClient:
    {
        return "TCP客户端";
    }
    }
    return "未知";
}

void NetStatusDataModel::resetCommunicationInfo(const CommunicationInfoMap& infoMap)
{
    records.clear();
    NetStatusDataInfo info;
    for (auto& item : infoMap.infoMap)
    {
        info.id = QString::number(item.id, 16);
        info.protocol = convetCommunicationTypeToDesc(item.type);
        info.recvPort = QString::number(item.recvPort);
        info.sendPort = QString::number(item.sendPort);
        info.localIP = item.localIP;
        info.desIP = item.desIP;
        info.desc = item.desc;
        records << info;
    }
}
void NetStatusDataModel::updateSendData(const NetStatusItemInfo& /*info*/)
{
    //    /* info.index 直接映射过来就是行 */
    //    if (info.index < 0 || info.index >= rowCount())
    //    {
    //        return;
    //    }
    //    // beginResetModel();
    //    auto& item = records[info.index];
    //    item.sendCount = QString::number(info.count);
    //    item.sendFailedCount = QString::number(info.failCount);
    //    item.sendState = info.state;
    //    item.sendData = info.data;
    //    QModelIndex t1 = index(info.index, 7);
    //    QModelIndex t2 = index(info.index, 8);
    //    emit dataChanged(t1, t2);
    // endResetModel();
}
void NetStatusDataModel::updateRecvData(const NetStatusItemInfo& /*info*/)
{
    //    /* info.index 直接映射过来就是行 */
    //    if (info.index < 0 || info.index >= rowCount())
    //    {
    //        return;
    //    }
    //    // beginResetModel();
    //    auto& item = records[info.index];
    //    item.recvCount = QString::number(info.count);
    //    item.recvFailedCount = QString::number(info.failCount);
    //    item.recvState = info.state;
    //    item.recvData = info.data;
    //    QModelIndex t1 = index(info.index, 10);
    //    QModelIndex t2 = index(info.index, 11);
    //    emit dataChanged(t1, t2);
    // endResetModel();
}
QVariant NetStatusDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int NetStatusDataModel::rowCount(const QModelIndex& /* parent */) const { return records.count(); }
int NetStatusDataModel::columnCount(const QModelIndex& /* parent */) const { return headerList.size(); }
static QString socketStateToString(QAbstractSocket::SocketState state)
{
    switch (state)
    {
    case QAbstractSocket::UnconnectedState: return "未连接";
    case QAbstractSocket::HostLookupState: return "正在查询主机";
    case QAbstractSocket::ConnectingState: return "开始连接";
    case QAbstractSocket::ConnectedState: return "已连接";
    case QAbstractSocket::BoundState: return "已绑定";
    case QAbstractSocket::ListeningState: return "仅内部使用";
    case QAbstractSocket::ClosingState: return "即将关闭";
    }
    return "未知状态";
}

QVariant NetStatusDataModel::data(const QModelIndex& index, int role) const
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
        case 0: return record.id;                              /* 通道ID */
        case 1: return record.desc;                            /* 描述 */
        case 2: return record.protocol;                        /* 通讯协议 */
        case 3: return record.localIP;                         /* 本地IP */
        case 4: return record.desIP;                           /* 目标IP */
        case 5: return record.recvPort;                        /* 接收端口 */
        case 6: return record.sendPort;                        /* 发送端口 */
        case 7: return record.sendCount;                       /* 发送包计数 */
        case 8: return record.sendFailedCount;                 /* 发送包错误计数 */
        case 9: return socketStateToString(record.sendState);  /* 发送链接状态 */
        case 10: return record.recvCount;                      /* 接收包计数 */
        case 11: return record.recvFailedCount;                /* 接收包错误计数 */
        case 12: return socketStateToString(record.recvState); /* 接收链接状态 */
        }
    }
    }
    return QVariant();
}

Qt::ItemFlags NetStatusDataModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

void NetStatusDataModel::updateByTimer(const QList<NetStatusDataInfo>& data)
{
    beginResetModel();
    records = data;
    endResetModel();
}

QList<NetStatusDataInfo> NetStatusDataModel::getRecords() const { return records; }
bool NetStatusDataModel::curDataInfo(const QModelIndex& index, NetStatusDataInfo& info)
{
    auto row = index.row();
    if (row < 0 || row > rowCount())
    {
        return false;
    }
    info = records.at(row);
    return true;
}
