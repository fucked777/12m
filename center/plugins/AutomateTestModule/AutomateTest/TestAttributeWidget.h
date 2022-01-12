#ifndef TESTATTRWIDGET_H
#define TESTATTRWIDGET_H

#include "AutomateTestDefine.h"
#include "SystemWorkMode.h"
#include "Utility.h"
#include <QByteArray>
#include <QMap>
#include <QWidget>

struct TestItemNodeData;
struct AutomateTestItem;

struct TestAttributeParameter
{
    QMap<QString, AutomateTestItemParameterInfo> infoMap;
    AutomateTestParameterBind automateTestParameterBind;
};

class TestParameterWidgetImpl;
class TestParameterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TestParameterWidget(QWidget* parent = nullptr);
    ~TestParameterWidget();
    void init(const AutomateTestItem& autoTestParam, SystemWorkMode workMode);
    void init(const TestItemNodeData& info);
    Optional<TestAttributeParameter> data();
    void setData(const AutomateTestPlanItem&);
    void clear();

private:
    TestParameterWidgetImpl* m_impl;
};

#endif  // TESTATTRWIDGET_H
