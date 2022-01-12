#include "AutomateTestFactory.h"
#include "AutomateTestBase.h"
#include <QDebug>
#include <QPen>
#include <QStringList>

class AutomateTestFactoryPrivate
{
public:
    struct AutomateTestTypeObjInfo
    {
        CreateTestContext createTestContext;
    };
    static QMap<QString, AutomateTestTypeObjInfo>& objMap()
    {
        static QMap<QString, AutomateTestTypeObjInfo> g_ObjMap;
        return g_ObjMap;
    }
    static QString createFindKey(const QString& typeID, SystemWorkMode workMode)
    {
        auto workModeStr = SystemWorkModeHelper::systemWorkModeToString(workMode);
        Q_ASSERT(!workModeStr.isEmpty());
        return QString("%1-%2").arg(typeID, workModeStr);
    }
};

void AutomateTestFactory::registerAutomateTestItemType(const QString& typeID, SystemWorkMode workMode, CreateTestContext func)
{
    auto key = AutomateTestFactoryPrivate::createFindKey(typeID, workMode);
    Q_ASSERT(!AutomateTestFactoryPrivate::objMap().contains(key));

    AutomateTestFactoryPrivate::AutomateTestTypeObjInfo info;
    info.createTestContext = func;
    AutomateTestFactoryPrivate::objMap().insert(key, info);
}
AutomateTestBase* AutomateTestFactory::createAutomateTestItem(const QString& typeID, SystemWorkMode workMode, AutomateTest* parent)
{
    auto key = AutomateTestFactoryPrivate::createFindKey(typeID, workMode);
    auto find = AutomateTestFactoryPrivate::objMap().find(key);
    if (find == AutomateTestFactoryPrivate::objMap().end())
    {
        return {};
    }
    return (find->createTestContext)(parent);
}

void AutomateTestFactory::destroyAutomateTestItem(AutomateTestBase*& base)
{
    if (base == nullptr)
    {
        return;
    }
    base->quit();
    delete base;
    base = nullptr;
}
