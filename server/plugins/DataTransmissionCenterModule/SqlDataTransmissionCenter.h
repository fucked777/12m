#ifndef SQLDATATRANSMISSIONCENTER_H
#define SQLDATATRANSMISSIONCENTER_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct DataTransmissionCenterData;
using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;
using DataTransmissionCenterList = QList<DataTransmissionCenterData>;
class SqlDataTransmissionCenter : public QObject
{
    Q_OBJECT
public:
    explicit SqlDataTransmissionCenter(QObject* parent = nullptr);
    ~SqlDataTransmissionCenter();

public:
    OptionalNotValue initDB();
    Optional<DataTransmissionCenterData> insertItem(const DataTransmissionCenterData& item);
    Optional<DataTransmissionCenterData> updateItem(const DataTransmissionCenterData& item);
    Optional<DataTransmissionCenterData> deleteItem(const DataTransmissionCenterData& item);
    Optional<DataTransmissionCenterMap> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLDATATRANSMISSIONCENTER_H
