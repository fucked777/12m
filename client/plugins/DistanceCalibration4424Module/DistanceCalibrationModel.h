#ifndef DISTANCECALIBRATIONMODEL_H
#define DISTANCECALIBRATIONMODEL_H

#include "DistanceCalibrationDefine.h"
#include <QAbstractListModel>
#include <QJsonObject>

class DistanceCalibrationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DistanceCalibrationModel(QObject* parent = Q_NULLPTR);
    void setValue(const DistanceCalibrationItemList&);
    void removeValue(const DistanceCalibrationItem&);
    void appendValue(const DistanceCalibrationItemList&);
    void clear();
    void updateData();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool itemAt(DistanceCalibrationItem& item, const QModelIndex& index);

private:
    DistanceCalibrationItemList records;
    QStringList headerList;
};

#endif  // DISTANCECALIBRATIONMODEL_H
