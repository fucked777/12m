#ifndef SQLEPHEMERISDATA_H
#define SQLEPHEMERISDATA_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct EphemerisDataData;
using EphemerisDataMap = QMap<QString, EphemerisDataData>;
using EphemerisDataList = QList<EphemerisDataData>;
class SqlEphemerisData : public QObject
{
    Q_OBJECT
public:
    explicit SqlEphemerisData(QObject* parent = nullptr);
    ~SqlEphemerisData();

public:
    OptionalNotValue initDB();
    Optional<EphemerisDataData> insertItem(const EphemerisDataData& item);
    Optional<EphemerisDataData> updateItem(const EphemerisDataData& item);
    Optional<EphemerisDataData> deleteItem(const EphemerisDataData& item);
    Optional<EphemerisDataMap> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLEPHEMERISDATA_H
