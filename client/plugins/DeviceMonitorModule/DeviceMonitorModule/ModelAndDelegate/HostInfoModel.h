#ifndef HOSTINFOMODEL_H
#define HOSTINFOMODEL_H

#include <QAbstractItemModel>

struct HostInfo{
    qint16 IP;
    QString name;
    qint32 status;
};

class HostInfoModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    HostInfoModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    void setData(const QList<HostInfo> &hostInfoList);
    QVariant data(const QModelIndex &index, int role) const;

    void setHeaderData(const QStringList &headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<HostInfo> mDataList;
};

#endif // HOSTINFOMODEL_H
