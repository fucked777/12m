#ifndef SQLDeviceStatusLogMANAGER_H
#define SQLDeviceStatusLogMANAGER_H

#include "Utility.h"
#include <QObject>

struct DeviceStatusRecord;
using DeviceStatusRecordList = QList<DeviceStatusRecord>;
class SqlDeviceStatusLogManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlDeviceStatusLogManager(QObject* parent = nullptr);
    ~SqlDeviceStatusLogManager();

public:
    OptionalNotValue initDB();
    Optional<bool> insertItem(const DeviceStatusRecord& item);
    Optional<bool> deleteAllItem();
    Optional<DeviceStatusRecordList> selectItem();

private:
    QString mDbConnectName;
};

#endif  // SQLDeviceStatusLogMANAGER_H
