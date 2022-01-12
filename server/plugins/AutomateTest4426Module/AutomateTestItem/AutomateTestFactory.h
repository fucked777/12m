#ifndef AUTOMATETESTFACTORY_H
#define AUTOMATETESTFACTORY_H

#include "SystemWorkMode.h"
#include "Utility.h"
#include <QMap>
#include <QString>
#include <functional>
#include <type_traits>

class AutomateTest;
class AutomateTestBase;
using CreateTestContext = std::function<AutomateTestBase*(AutomateTest*)>;

class AutomateTestFactory
{
public:
    static void registerAutomateTestItemType(const QString& typeID, SystemWorkMode workMode, CreateTestContext func);
    static AutomateTestBase* createAutomateTestItem(const QString& typeID, SystemWorkMode workMode, AutomateTest* parent);
    static void destroyAutomateTestItem(AutomateTestBase*);
};

#endif  // AUTOMATETESTFACTORY_H
