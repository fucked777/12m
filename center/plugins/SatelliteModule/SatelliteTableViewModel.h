#ifndef SATELLITETABLEVIEWMODEL_H
#define SATELLITETABLEVIEWMODEL_H

#include <QAbstractTableModel>
struct SatelliteManagementData;
class SatelliteTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SatelliteTableViewModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    void setHorizontalHeader(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setSatelliteData(const QList<SatelliteManagementData>& data);
    QList<SatelliteManagementData>& DataVector();

    QString satelliteID(const QModelIndex index);
    SatelliteManagementData SatelliteData(const QModelIndex index);
    ~SatelliteTableViewModel(void);

    void setWorkModeMap(const QMap<int, QString>& workModeMap);

signals:

public slots:

private:
    QStringList m_HorizontalHeader;
    QList<SatelliteManagementData> m_data;
    QMap<int, QString> m_workModeMap;
    QString m_currentProject;  //当前项目  4424 or  4426
};
#endif  // SatelliteTABLEVIEWMODEL_H
