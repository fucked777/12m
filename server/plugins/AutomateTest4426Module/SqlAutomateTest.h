#ifndef SQLAUTOMATETEST_H
#define SQLAUTOMATETEST_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct AutomateTestResultQuery;
struct AutomateTestParameterBind;
struct AutomateTestPlanItem;
using AutomateTestPlanItemList = QList<AutomateTestPlanItem>;
struct AutomateTestParameterBindGroup;
using AutomateTestParameterBindMap = QMap<QString, AutomateTestParameterBindGroup>;
struct AutomateTestResult;
using AutomateTestResultList = QList<AutomateTestResult>;

class SqlAutomateTest : public QObject
{
public:
    static OptionalNotValue initDB();
    /* 测试计划 */
    static Optional<AutomateTestPlanItem> insertTestItem(const AutomateTestPlanItem& item);
    static Optional<AutomateTestPlanItemList> insertTestItem(const AutomateTestPlanItemList& item);
    static Optional<AutomateTestPlanItem> deleteTestItem(const AutomateTestPlanItem& item);
    static Optional<AutomateTestPlanItem> deleteHistoryTestItem(const AutomateTestPlanItem& item);
    static Optional<AutomateTestPlanItemList> queryTestItem();

    /* 测试参数装订 */
    static Optional<AutomateTestParameterBind> updateTestBind(const AutomateTestParameterBind& item);
    static Optional<AutomateTestParameterBind> deleteTestBind(const AutomateTestParameterBind& item);
    static Optional<AutomateTestParameterBindMap> queryTestBind();

    /* 测试结果 */
    static Optional<AutomateTestResult> insertTestResultItem(const AutomateTestResult& item);
    static Optional<AutomateTestResult> deleteTestResultItem(const AutomateTestResult& item);
    static Optional<AutomateTestResultList> queryTestResultItem(const AutomateTestResultQuery&);

private:
    static OptionalNotValue initTestItemDB(QSqlDatabase&);   /* 测试计划 */
    static OptionalNotValue initTestBindDB(QSqlDatabase&);   /* 测试参数绑定 */
    static OptionalNotValue initTestResultDB(QSqlDatabase&); /* 测试结果 */
};

#endif  // SQLAUTOMATETEST_H
