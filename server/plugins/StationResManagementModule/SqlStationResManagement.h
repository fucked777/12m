#ifndef SQLSTATIONRESMANAGEMENT_H
#define SQLSTATIONRESMANAGEMENT_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct StationResManagementData;
using StationResManagementDataMap = QMap<QString, StationResManagementData>;
using StationResManagementDataList = QList<StationResManagementData>;
class SqlStationResManagement : public QObject
{
    Q_OBJECT
public:
    explicit SqlStationResManagement(QObject* parent = nullptr);
    ~SqlStationResManagement();

public:
    OptionalNotValue initDB();
    Optional<StationResManagementData> insertItem(const StationResManagementData& item);
    Optional<StationResManagementData> updateItem(const StationResManagementData& item);
    Optional<StationResManagementData> deleteItem(const StationResManagementData& item);
    Optional<StationResManagementDataMap> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLSTATIONRESMANAGEMENT_H
