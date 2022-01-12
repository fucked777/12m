#ifndef SQLSatelliteMANAGER_H
#define SQLSatelliteMANAGER_H

#include "Utility.h"
#include <QList>
#include <QObject>

struct SatelliteManagementData;

class SqlSatelliteManager
{
public:
    explicit SqlSatelliteManager();
    ~SqlSatelliteManager();

public:
    OptionalNotValue initDB();
    OptionalNotValue insertItem(SatelliteManagementData& item);
    OptionalNotValue updateItem(SatelliteManagementData& item);
    OptionalNotValue deleteItem(const SatelliteManagementData& item);
    Optional<QList<SatelliteManagementData>> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLSatelliteMANAGER_H
