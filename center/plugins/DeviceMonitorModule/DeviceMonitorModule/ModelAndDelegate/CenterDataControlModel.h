#ifndef CENTERDATACONTROLMODEL_H
#define CENTERDATACONTROLMODEL_H
#include <QAbstractItemModel>

struct CenterDataControl{
    QString id;
    QString code;
    qint32 isUing;
    qint32 isSendTtcFirstRouter;
    qint32 isSendLinkFirstRouter;
    qint32 isSendTtcSecondRouter;
    qint32 isSendLinkSecondRouter;
};

class CenterDataControlModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    CenterDataControlModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    void setData(const QList<CenterDataControl> &centerDataControlList);
    QVariant data(const QModelIndex &index, int role) const;

    void setHeaderData(const QStringList &headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<CenterDataControl> mDataList;

};

#endif // CENTERDATACONTROLMODEL_H
