#ifndef SQLCONFIGMACROMANAGER_H
#define SQLCONFIGMACROMANAGER_H

#include "Utility.h"
#include <QObject>

struct ConfigMacroData;
using ConfigMacroDataList = QList<ConfigMacroData>;
class SqlConfigMacroManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlConfigMacroManager(QObject* parent = nullptr);
    ~SqlConfigMacroManager();

public:
    OptionalNotValue initDB();
    Optional<ConfigMacroData> insertItem(const ConfigMacroData& item);
    Optional<ConfigMacroData> updateItem(const ConfigMacroData& item);
    Optional<ConfigMacroData> deleteItem(const ConfigMacroData& item);
    Optional<ConfigMacroDataList> selectItem();

private:
    QString mDbConnectName;
};

#endif  // SQLCONFIGMACROMANAGER_H
