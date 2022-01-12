#ifndef NETCARDMODEL_H
#define NETCARDMODEL_H

#include <QAbstractItemModel>

struct NetCardInfo
{
    QString IP;
    qint32 hostId;
    qint32 type;
    qint32 status;
};

class NetCardModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    NetCardModel(QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;

    void setData(const QList<NetCardInfo>& netCardInfoList);
    QVariant data(const QModelIndex& index, int role) const;

    void setHeaderData(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<NetCardInfo> mDataList;
};

#endif  // NETCARDMODEL_H
