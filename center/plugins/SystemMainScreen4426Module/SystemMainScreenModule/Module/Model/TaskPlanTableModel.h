#ifndef TASKPLANTABLEMODEL_H
#define TASKPLANTABLEMODEL_H

#include <QAbstractTableModel>

class TaskPlanTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TaskPlanTableModel(QObject* parent = nullptr);
    ~TaskPlanTableModel();

    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override;

private:
    QStringList mHeaders;
};

#endif  // TASKPLANTABLEMODEL_H
