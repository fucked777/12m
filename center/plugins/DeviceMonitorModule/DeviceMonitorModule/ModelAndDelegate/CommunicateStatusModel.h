#ifndef COMMUNICATESTATUSMODEL_H
#define COMMUNICATESTATUSMODEL_H

#include <QAbstractItemModel>

struct CommunicateStatus
{
    qint32 portId;
    QString portCode;
    qint32 bId;  // 数据类型标识
    QString dataCode;
    QString mId;  // 任务标识
    QString uac;
    qint32 framesNum;
    qint32 dir;
};

class CommunicateStatusModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    CommunicateStatusModel(QObject* parent = nullptr);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;

    void setData(const QList<CommunicateStatus>& commStatusList);
    QVariant data(const QModelIndex& index, int role) const;

    void setHeaderData(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<CommunicateStatus> mDataList;
};

#endif  // COMMUNICATESTATUSMODEL_H
