#ifndef TASKRUNCENTERDATAMODEL_H
#define TASKRUNCENTERDATAMODEL_H

#include <QAbstractItemModel>

struct TaskRunCenterData
{
    QString taskRunCenterBZ;       //任务参试中心标识
    qint32 taskRunCenterSSStatus;  //任务参试中心送数状态
};

class TaskRunCenterDataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TaskRunCenterDataModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setHeaderData(const QStringList& headers);

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void setData(const QList<TaskRunCenterData>& softUnitHostList);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QStringList m_headerList;
    QList<TaskRunCenterData> m_dataList;
};

#endif  // TASKRUNCENTERDATAMODEL_H
