#ifndef PHASECALIBRATIONMODEL_H
#define PHASECALIBRATIONMODEL_H

#include "PhaseCalibrationDefine.h"
#include <QAbstractListModel>
#include <QJsonObject>

class PhaseCalibrationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PhaseCalibrationModel(QObject* parent = Q_NULLPTR);
    void setValue(const PhaseCalibrationItemList&);
    void removeValue(const PhaseCalibrationItem&);
    void clear();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool itemAt(const QModelIndex& index, PhaseCalibrationItem& item);

private:
    PhaseCalibrationItemList records;
    QStringList headerList;
};

#endif  // PHASECALIBRATIONMODEL_H
