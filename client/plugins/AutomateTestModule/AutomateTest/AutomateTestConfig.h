#ifndef AUTOMATETESTCONFIG_H
#define AUTOMATETESTCONFIG_H
#include "DynamicConfigToWidgetUtility.h"
#include "SystemWorkMode.h"
#include "Utility.h"
#include <QGroupBox>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <tuple>
#include <type_traits>

/* 单个测试项信息 */
struct AutomateTestItem
{
    QString id;                                       /* 测试项索引的唯一ID */
    QString name;                                     /* 测试项的名称 */
    QMap<SystemWorkMode, QString> workModeMapToParam; /* 工作模式和对应的参数的映射,没有就找不到 */
    /* 不为空代表有参数绑定,值为参数绑定的配置文件路径 */
    QString bind;

    // int type{ 0 }; /* 待定 */

    /* 是否支持当前传入的工作模式 */
    bool workModeEnable(SystemWorkMode workMode) { return workModeMapToParam.contains(workMode); }
};
/* 测试项分组 */
struct AutomateTestGroup
{
    QString groupName;
    QList<AutomateTestItem> itemList;
};

/* 测试的项目信息 */
struct AutomateTestItemInfo
{
    QMap<QString, AutomateTestItem> itemMap; /* 用于索引 */
    QList<AutomateTestGroup> groupList;      /* 用于生成界面 */
};

struct TestSingleParameterGroup
{
    QString groupName;                           /* 分组的名字 */
    QList<CommonToWidgetInfo> testItemParameter; /* 这个是测试项的参数 */
};

/* 单个测试项的参数 */
struct TestSingleParameter
{
    QList<TestSingleParameterGroup> testSingleParameterGroup; /* 这个是测试项的参数 */
    /* 结果参数 */
    /* 参数装订参数 */
};

Q_DECLARE_METATYPE(TestSingleParameterGroup);
Q_DECLARE_METATYPE(TestSingleParameter);
Q_DECLARE_METATYPE(AutomateTestItem);
Q_DECLARE_METATYPE(AutomateTestGroup);
Q_DECLARE_METATYPE(AutomateTestItemInfo);

class QDomElement;
class AutomateTestConfig
{
public:
    /* 解析配置文件获取测试项的数据 */
    static Optional<AutomateTestItemInfo> parseTestItemConfig();
    /* 解析单个测试项的配置文件 */
    static Optional<TestSingleParameter> parseSingleTestItem(const QString& filePath);

private:
    static void parseTestItemConfigImpl(QDomElement& element, AutomateTestItemInfo& testItemInfo, int& index);
    static OptionalNotValue parseSingleTestItemImpl(QDomElement& element, TestSingleParameter& testItemParameter, int& index);
};

#endif  // AUTOMATETESTCONFIG_H
