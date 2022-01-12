#ifndef NETSTATUSDATAMODEL_H
#define NETSTATUSDATAMODEL_H

#include <QAbstractListModel>
#include <QAbstractSocket>

struct CommunicationInfoMap;
// using CommunicationInfoMap = QMap<quint32, CommunicationInfo>;
struct NetStatusItemInfo;
struct NetStatusDataInfo
{
    QString id;       /* 通道ID */
    QString desc;     /* 描述 */
    QString protocol; /* 通讯协议 */
    QString localIP;  /* 本地IP */
    QString desIP;    /* 目标IP */
    QString recvPort; /* 接收端口 */
    QString sendPort; /* 发送端口 */

    QString sendCount{ "-" };                                                    /* 发送包计数 */
    QString sendFailedCount{ "-" };                                              /* 发送包错误计数 */
    QAbstractSocket::SocketState sendState{ QAbstractSocket::UnconnectedState }; /* 发送链接状态 */
    QString recvCount{ "-" };                                                    /* 接收包计数 */
    QString recvFailedCount{ "-" };                                              /* 接收包错误计数 */
    QAbstractSocket::SocketState recvState{ QAbstractSocket::UnconnectedState }; /* 接收链接状态 */

    QByteArray recvData; /* 接收的数据 */
    QByteArray sendData; /* 发送的数据 */
};

class NetStatusDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit NetStatusDataModel(QObject* parent = Q_NULLPTR);
    /* 重置界面显示数据,不包括接收和发送的数据 */
    void resetCommunicationInfo(const CommunicationInfoMap&);
    void updateSendData(const NetStatusItemInfo&);
    void updateRecvData(const NetStatusItemInfo&);

    bool curDataInfo(const QModelIndex& index, NetStatusDataInfo& info);

    void updateData();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void updateByTimer(const QList<NetStatusDataInfo>& data);

    QList<NetStatusDataInfo> getRecords() const;

private:
    QList<NetStatusDataInfo> records;
    QStringList headerList;
};

#endif  // NETSTATUSDATAMODEL_H
