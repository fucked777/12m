#ifndef SQLResourceRestructuringMANAGER_H
#define SQLResourceRestructuringMANAGER_H

#include "Utility.h"
#include <QObject>

struct ResourceRestructuringData;
using ResourceRestructuringDataList = QList<ResourceRestructuringData>;
class SqlResourceRestructuringManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlResourceRestructuringManager(QObject* parent = nullptr);
    ~SqlResourceRestructuringManager();

public:
    OptionalNotValue initDB();
    Optional<ResourceRestructuringData> insertItem(const ResourceRestructuringData& item);
    Optional<ResourceRestructuringData> updateItem(const ResourceRestructuringData& item);
    Optional<ResourceRestructuringData> deleteItem(const ResourceRestructuringData& item);
    Optional<ResourceRestructuringDataList> selectItem();

private:
    QString mDbConnectName;
};

#endif  // SQLResourceRestructuringMANAGER_H
