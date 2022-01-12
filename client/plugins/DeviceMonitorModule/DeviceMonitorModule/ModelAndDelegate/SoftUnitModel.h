#ifndef SOFTUNITMODEL_H
#define SOFTUNITMODEL_H

#include <QAbstractItemModel>

struct SoftUnit{
    qint16 id;
    QString code;
    qint32 status;
    qint32 detailNum;
    qint32 hostId;
    qint32 detailStatus;
};

class SoftUnitModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    SoftUnitModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    void setData(const QList<SoftUnit> &softUnitList);
    QVariant data(const QModelIndex &index, int role) const;

    void setHeaderData(const QStringList &headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<SoftUnit> mDataList;

};

#endif // SOFTUNITMODEL_H
