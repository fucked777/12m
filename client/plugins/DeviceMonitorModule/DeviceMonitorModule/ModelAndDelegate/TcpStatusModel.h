#ifndef TCPSTATUSMODEL_H
#define TCPSTATUSMODEL_H

#include <QAbstractItemModel>

struct TcpStatus{
    qint16 id;
    qint32 bbeIndex;
    qint32 status;
};

class TcpStatusModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    TcpStatusModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    void setData(const QList<TcpStatus> &tcpStatusList);
    QVariant data(const QModelIndex &index, int role) const;

    void setHeaderData(const QStringList &headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<TcpStatus> mDataList;

};

#endif // TCPSTATUSMODEL_H
