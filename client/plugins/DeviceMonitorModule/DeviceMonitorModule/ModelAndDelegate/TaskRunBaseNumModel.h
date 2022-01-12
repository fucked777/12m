#ifndef TASKRUNBASENUMMODEL_H
#define TASKRUNBASENUMMODEL_H

#include <QAbstractItemModel>

struct TaskRunBaseNum
{
    QString taskRunBaseBZ;  //任务参试基带标识
};

class TaskRunBaseNumModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TaskRunBaseNumModel(QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setHeaderData(const QStringList& headers);

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void setData(const QList<TaskRunBaseNum>& softUnitHostList);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QStringList m_headerList;
    QList<TaskRunBaseNum> m_dataList;
};

#endif  // TASKRUNBASENUMMODEL_H
