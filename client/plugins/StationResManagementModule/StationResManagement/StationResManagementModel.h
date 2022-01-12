#ifndef STATIONRESMANAGEMENTMODEL_H
#define STATIONRESMANAGEMENTMODEL_H

#include "Utility.h"
#include <QAbstractTableModel>
struct StationResManagementData;
using StationResManagementMap = QMap<QString, StationResManagementData>;
using StationResManagementList = QList<StationResManagementData>;

class StationResManagementModelImpl;
class StationResManagementModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StationResManagementModel(QObject* parent = nullptr);
    ~StationResManagementModel() override;

    void addItem(const StationResManagementData& value);
    void updateItem(const StationResManagementData&);
    void deleteItem(const QModelIndex& index);
    void deleteItem(const StationResManagementData&);
    void updateData(const StationResManagementList& logs);
    void updateData(const StationResManagementMap& logs);
    void updateData();
    bool judgeData(int row);
    bool dataExist(const StationResManagementData&);
    Optional<StationResManagementData> getDataCenterItem(const QModelIndex&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    StationResManagementModelImpl* m_impl;
};

#endif  // STATIONRESMANAGEMENTMODEL_H
