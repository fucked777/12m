#ifndef DATATRANSMISSIONMODEL_H
#define DATATRANSMISSIONMODEL_H

#include "Utility.h"
#include <QAbstractTableModel>
struct DataTransmissionCenterData;
using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;
using DataTransmissionCenterList = QList<DataTransmissionCenterData>;

class DataTransmissionModelImpl;
class DataTransmissionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DataTransmissionModel(QObject* parent = nullptr);
    ~DataTransmissionModel() override;

    void addItem(const DataTransmissionCenterData& value);
    void updateItem(const DataTransmissionCenterData&);
    void deleteItem(const QModelIndex& index);
    void deleteItem(const DataTransmissionCenterData&);
    void updateData(const DataTransmissionCenterList& logs);
    void updateData(const DataTransmissionCenterMap& logs);
    void updateData();
    bool judgeData(int row);
    bool dataExist(const DataTransmissionCenterData&);
    Optional<DataTransmissionCenterData> getDataCenterItem(const QModelIndex&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    DataTransmissionModelImpl* m_impl;
};

#endif  // DATATRANSMISSIONMODEL_H
