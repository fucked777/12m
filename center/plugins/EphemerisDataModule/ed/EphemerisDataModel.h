#ifndef EPHEMERISDATAMODEL_H
#define EPHEMERISDATAMODEL_H

#include "Utility.h"
#include <QAbstractTableModel>
struct EphemerisDataData;
using EphemerisDataMap = QMap<QString, EphemerisDataData>;
using EphemerisDataList = QList<EphemerisDataData>;

class EphemerisDataModelImpl;
class EphemerisDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit EphemerisDataModel(QObject* parent = nullptr);
    ~EphemerisDataModel() override;

    void addItem(const EphemerisDataData& value);
    void updateItem(const EphemerisDataData&);
    void deleteItem(const QModelIndex& index);
    void deleteItem(const EphemerisDataData&);
    void updateData(const EphemerisDataList& logs);
    void updateData(const EphemerisDataMap& logs);
    void updateData();
    bool judgeData(int row);
    bool dataExist(const EphemerisDataData&);
    Optional<EphemerisDataData> getItem(const QModelIndex&);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    EphemerisDataModelImpl* m_impl;
};

#endif  // EPHEMERISDATAMODEL_H
