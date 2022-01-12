#ifndef AutomateTestUtility_H
#define AutomateTestUtility_H
#include "AutomateTestConfig.h"
#include "AutomateTestDefine.h"
#include "DynamicConfigToWidgetUtility.h"
#include "TestAttributeWidget.h"
#include "Utility.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <tuple>

/* 每一个测试项的数据 */
struct TestItemNodeData
{
public:
    AutomateTestItem autoTestParam; /* 当前测试项的测试参数 */
    QString groupName;              /* 测试的组名称/就是分系统名称 */
    /* 设置参数的界面 */
    TestParameterWidget* testParameterWidget{ nullptr };
    SystemWorkMode workMode{ SystemWorkMode::NotDefine }; /* 当前界面是什么工作模式创建的 */
};
Q_DECLARE_METATYPE(TestItemNodeData);

/* 未知参数 */
class QLabel;
class NoneParam : public QWidget
{
public:
    NoneParam(QWidget* parent = nullptr);
    ~NoneParam() override;
    void setText(const QString&);

private:
    QLabel* m_text;
};

struct ParameterBindingConfigInfo
{
    /* 参数绑定的参数 */
    QList<CommonToWidgetInfo> testItemParameter;
    /* 参数绑定的列表标题 0是key 1是标题 */
    QList<std::tuple<QString, QString>> headList;
};
/*
 * 20210823 wp??
 * 此处发现部分测试项可能会根据模式不同参数不同
*/
using ParameterBindingConfigMap = QMap<QString,ParameterBindingConfigInfo>;
struct ParameterBindingConfigRelation
{
    QString relation;
    /* key是combox的文本 */
    ParameterBindingConfigMap configMap;
};



class AutomateTestWidget;
class AutomateTestUtility
{
public:
    static AutomateTestParameterBindMap queryParameterBind();
    static Optional<ParameterBindingConfigRelation> parseParameterBindingConfig(const AutomateTestItem& testItem);

private:
    static OptionalNotValue parseParameterBindingConfigImpl(const QStringList&valueList, QDomElement& root, ParameterBindingConfigRelation&);
};

#endif  // AutomateTestUtility_H
