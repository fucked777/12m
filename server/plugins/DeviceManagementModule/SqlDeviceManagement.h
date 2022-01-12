#ifndef SQLDEVICEMANAGEMENT_H
#define SQLDEVICEMANAGEMENT_H

#include "Utility.h"
#include <QList>
#include <QObject>

struct DMQuery;
struct DMDataItem;
using DMTypeMap = QMap<QString, QList<DMDataItem>>;
using DMTypeList = QList<DMDataItem>;
class SqlDeviceManagement
{
public:
    explicit SqlDeviceManagement();
    ~SqlDeviceManagement();

public:
    OptionalNotValue initDB();
    Optional<DMDataItem> insertItem(const DMDataItem& item);
    Optional<DMDataItem> updateItem(const DMDataItem& item);
    Optional<DMDataItem> deleteItem(const DMDataItem& item);
    Optional<DMTypeMap> selectItem(const DMQuery& query);
    Optional<DMTypeMap> selectItem();

private:
    QString m_dbConnectName;
};

#endif  // SQLDEVICEMANAGER_H
