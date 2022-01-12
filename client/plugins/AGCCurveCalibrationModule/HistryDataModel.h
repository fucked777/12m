#ifndef HISTRYDATAMODEL_H
#define HISTRYDATAMODEL_H
#include "AGCDefine.h"
#include <QAbstractListModel>
#include <QJsonObject>

class HistryDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit HistryDataModel(QObject* parent = Q_NULLPTR);
    void updateData(const AGCCalibrationItemList&);
    void appendData(const AGCCalibrationItemList&);
    void removeValue(const AGCCalibrationItem& removeItem);
    void clear();
    void updateData();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool agcItemData(const QModelIndex& index, AGCCalibrationItem&);
    bool isEmpty() const { return records.isEmpty(); }

private:
    AGCCalibrationItemList records;
    QStringList headerList;
};

#endif  // HISTRYDATAMODEL_H
