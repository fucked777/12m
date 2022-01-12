#ifndef TASKCENTERMODEL_H
#define TASKCENTERMODEL_H

#include "Utility.h"
#include <QAbstractTableModel>
struct TaskCenterData;
using TaskCenterMap = QMap<QString, TaskCenterData>;
using TaskCenterList = QList<TaskCenterData>;

class TaskCenterModelImpl;
class TaskCenterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TaskCenterModel(QObject* parent = nullptr);
    ~TaskCenterModel() override;

    void addItem(const TaskCenterData& value);
    void updateItem(const TaskCenterData&);
    void deleteItem(const QModelIndex& index);
    void deleteItem(const TaskCenterData&);
    void updateData(const TaskCenterList& logs);
    void updateData(const TaskCenterMap& logs);
    void updateData();
    bool judgeData(int row);
    bool dataExist(const TaskCenterData&);
    Optional<TaskCenterData> getDataCenterItem(const QModelIndex&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    TaskCenterModelImpl* m_impl;
};

#endif  // TASKCENTERMODEL_H
