#ifndef SQLParamMacroMANAGER_H
#define SQLParamMacroMANAGER_H

#include "Utility.h"
#include <QList>
#include <QObject>

struct ParamMacroData;
using ParamMacroDataList = QList<ParamMacroData>;

class SqlParamMacroManager
{
public:
    explicit SqlParamMacroManager();
    ~SqlParamMacroManager();

public:
    OptionalNotValue initDB();

    Optional<ParamMacroData> insertItem(const ParamMacroData& item);
    Optional<ParamMacroData> updateItem(const ParamMacroData& item);
    Optional<ParamMacroData> deleteItem(const ParamMacroData& item);
    Optional<ParamMacroDataList> selectItem();

private:
    QString mDbConnectName;
};

#endif  // SQLParamMacroMANAGER_H
