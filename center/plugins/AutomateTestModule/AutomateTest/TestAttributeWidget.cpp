#include "TestAttributeWidget.h"
#include "AutomateTestUtility.h"
#include "DynamicConfigToWidgetUtility.h"
#include "GlobalData.h"
#include "ParameterBindingModel.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMap>
#include <QScrollArea>
#include <QTableView>
#include <QVBoxLayout>

class TestParameterWidgetImpl
{
public:
    AutomateTestItem autoTestParam;
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };
    TestSingleParameter testSingleParameter;
    QMap<QString, DynamicWidgetInfo> dataWidget; /* key为json值的key value为key对应的值的控件 */
    QTableView* tabeView{ nullptr };
    bool notSupported{ false }; /* 当前模式是否支持此测试项 */
    QString tipsMsg;
    ParameterBindingModel model;
    QVBoxLayout* mainLayout{ nullptr };
    QScrollArea* scrollArea{ nullptr };

    static void createEmptyWidget(const QString& desc, QWidget* parent)
    {
        auto layout = new QVBoxLayout(parent);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        auto lable = new QLabel(parent);
        lable->setText(desc);
        layout->addStretch();
        layout->addWidget(lable, 0, Qt::AlignCenter);
        layout->addStretch();
    }

    void initTestPara(const QList<TestSingleParameterGroup>& contentList, QVBoxLayout* vLayout, QWidget* parent);
    void initBindPara(const AutomateTestItem& autoTestParam, QVBoxLayout* vLayout, QWidget* parent);
    void refreshBind()
    {
        AutomateTestParameterBindMap tempMap;
        if (!GlobalData::getAutomateTestParameterBindMap(tempMap))
        {
            return;
        }
        auto value = tempMap.value(autoTestParam.id);
        /* 过滤一下工作模式 */
        QList<AutomateTestParameterBind> filterValue;
        for (auto& item : value.bindDataList)
        {
            if (item.workMode == workMode)
            {
                filterValue << item;
            }
        }
        model.updateData(filterValue);
    }
};

/* 初始化测试的参数部分 */
void TestParameterWidgetImpl::initTestPara(const QList<TestSingleParameterGroup>& contentList, QVBoxLayout* vLayout, QWidget* parent)
{
    auto widget = new QWidget(parent);
    auto layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);

    for (auto& item : contentList)
    {
        int row = 0;
        int col = 0;

        auto groupBox = new QGroupBox(widget);
        auto widgetLayout = new QGridLayout(groupBox);

        groupBox->setTitle(item.groupName);
        widgetLayout->setContentsMargins(0, 15, 0, 10);
        widgetLayout->setHorizontalSpacing(10);
        widgetLayout->setVerticalSpacing(20);
        for (auto& widgetItem : item.testItemParameter)
        {
            auto widgetResult = DynamicConfigToWidgetUtility::createWidget(widgetItem);
            if (!widgetResult || widgetResult->widget == nullptr)
            {
                continue;
            }
            auto& dynamicWidgetInfo = widgetResult.value();
            dynamicWidgetInfo.widget->setParent(groupBox);

            auto label = new QLabel(widget);
            label->setText(widgetItem.name + ":");

            label->setFixedWidth(150);
            dynamicWidgetInfo.widget->setFixedWidth(200);

            widgetLayout->addWidget(label, row, col++);
            widgetLayout->addWidget(widgetResult->widget, row, col++);
            if (col >= 4)
            {
                col = 0;
                ++row;
            }
            dataWidget[widgetItem.id] = dynamicWidgetInfo;
        }
        layout->addWidget(groupBox);
    }

    vLayout->addWidget(widget);
}
void TestParameterWidgetImpl::initBindPara(const AutomateTestItem& autoTestParam, QVBoxLayout* vLayout, QWidget* parent)
{
    if (autoTestParam.bind.isEmpty())
    {
        vLayout->addStretch();
        return;
    }
    auto result = AutomateTestUtility::parseParameterBindingConfig(autoTestParam);
    if (!result)
    {
        vLayout->addStretch();
        qWarning() << result.msg();
        return;
    }
    auto& parameterBindingConfigRelation = result.value();
    if(parameterBindingConfigRelation.configMap.isEmpty())
    {
        vLayout->addStretch();
        return;
    }
    auto parameterBindingConfigInfo = parameterBindingConfigRelation.configMap.first();

    model.setIndexData(parameterBindingConfigInfo);

    tabeView = new QTableView(parent);
    Utility::tableViewInit(tabeView, true);
    tabeView->setModel(&model);

    vLayout->addWidget(tabeView);
}
TestParameterWidget::TestParameterWidget(QWidget* parent)
    : QWidget(parent)
    , m_impl(new TestParameterWidgetImpl)
{
    m_impl->notSupported = true;
    m_impl->tipsMsg = "当前未设置参数";
}

TestParameterWidget::~TestParameterWidget() { delete m_impl; }

void TestParameterWidget::init(const AutomateTestItem& autoTestParam, SystemWorkMode workMode)
{
    m_impl->autoTestParam = autoTestParam;
    m_impl->workMode = workMode;
    auto find = autoTestParam.workModeMapToParam.find(workMode);
    /* 当前模式不支持测试 */
    if (find == autoTestParam.workModeMapToParam.end())
    {
        m_impl->notSupported = true;
        m_impl->tipsMsg = "当前测试项不支持此模式参数";
        TestParameterWidgetImpl::createEmptyWidget(m_impl->tipsMsg, this);
        return;
    }
    auto parseResult = AutomateTestConfig::parseSingleTestItem(find.value());
    if (!parseResult)
    {
        m_impl->notSupported = true;
        m_impl->tipsMsg = QString("测试参数解析失败\n%1").arg(parseResult.msg());
        TestParameterWidgetImpl::createEmptyWidget(m_impl->tipsMsg, this);
        return;
    }
    m_impl->testSingleParameter = parseResult.value();
    if (m_impl->testSingleParameter.testSingleParameterGroup.isEmpty())
    {
        m_impl->notSupported = false; /* 这个是本来就没有参数 */
        TestParameterWidgetImpl::createEmptyWidget("当前测试项无设置参数", this);
        return;
    }

    m_impl->notSupported = false;
    m_impl->scrollArea = new QScrollArea(this);
    m_impl->scrollArea->setWidgetResizable(true);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->setSpacing(30);
    layout->setContentsMargins(0, 10, 0, 0);

    m_impl->initTestPara(m_impl->testSingleParameter.testSingleParameterGroup, layout, widget);

    m_impl->initBindPara(autoTestParam, layout, widget);

    m_impl->scrollArea->setWidget(widget);

    m_impl->mainLayout = new QVBoxLayout(this);
    m_impl->mainLayout->setSpacing(0);
    m_impl->mainLayout->setContentsMargins(0, 0, 0, 0);
    m_impl->mainLayout->addWidget(m_impl->scrollArea);

    m_impl->refreshBind();
}

void TestParameterWidget::init(const TestItemNodeData& info) { init(info.autoTestParam, info.workMode); }
Optional<TestAttributeParameter> TestParameterWidget::data()
{
    using ResType = Optional<TestAttributeParameter>;
    if (m_impl->notSupported)
    {
        return ResType(ErrorCode::NotSupported, m_impl->tipsMsg);
    }
    TestAttributeParameter testAttributeParameter;
    for (auto& item : m_impl->dataWidget)
    {
        auto varResult = DynamicConfigToWidgetUtility::getWidgetValue(item);
        if (!varResult)
        {
            return ResType(varResult.errorCode(), varResult.msg());
        }
        AutomateTestItemParameterInfo info;
        info.displayName = item.widgetInfo.name;
        info.rawValue = varResult.value();
        info.id = item.widgetInfo.id;
        if (item.widgetInfo.type == DynamicConfigToWidgetType::Enum)
        {
            auto temp = qobject_cast<QComboBox*>(item.widget);
            info.displayValue = temp->currentText();
        }
        else
        {
            info.displayValue = info.rawValue.toString();
        }
        testAttributeParameter.infoMap.insert(info.id, info);

        /* 不为nullptr就是有绑定参数的 */
        if (m_impl->tabeView != nullptr)
        {
            if (!m_impl->model.itemAt(testAttributeParameter.automateTestParameterBind, m_impl->tabeView->currentIndex()))
            {
                return ResType(ErrorCode::NotFound, QString("请选择%1的装订数据").arg(m_impl->autoTestParam.name));
            }
        }
    }

    return ResType(testAttributeParameter);
}
void TestParameterWidget::setData(const AutomateTestPlanItem& item)
{
    for (auto& widgetInfo : m_impl->dataWidget)
    {
        auto find = item.testParameterMap.find(widgetInfo.widgetInfo.id);
        if (find == item.testParameterMap.end())
        {
            continue;
        }
        DynamicConfigToWidgetUtility::setWidgetValue(widgetInfo, find->rawValue);
    }
    if (!item.automateTestParameterBind.testBindParameterMap.isEmpty())
    {
        m_impl->model.updateData(item.automateTestParameterBind);
    }
}

// bool TestParameterWidget::bindData(CalibrationItem& scItem, QJsonObject& jsonObject, QString& errMsg)
//{
//    if (m_impl->mode == nullptr)
//    {
//        return true;
//    }
//    if (!m_impl->mode->dataCheck(scItem, jsonObject, m_impl->tabeView->currentIndex(), errMsg))
//    {
//        return false;
//    }
//    errMsg = "请选择一个参数装订";
//    return m_impl->mode->item(scItem.other, jsonObject, m_impl->tabeView->currentIndex());
//}
// void TestParameterWidget::fillSystemCalibrationItem(CalibrationItem& scItem)
//{
//    if (m_impl->mode == nullptr)
//    {
//        return;
//    }
//    m_impl->mode->fillSystemCalibrationItem(scItem, m_impl->tabeView->currentIndex());
//}

void TestParameterWidget::clear()
{
    //    for (auto iter = m_impl->dataWidget.begin(); iter != m_impl->dataWidget.end(); ++iter)
    //    {
    //        AutoTestHelper::resetValue(iter.value());
    //    }
}

// class TestViewWidgetImpl
//{
// public:
//    // CmdFindStruct cmdFind;
//    QMap<QString, QWidget*> dataWidget; /* key为json值的key value为key对应的值的控件 */
//    QTableView* tabeView{ nullptr };
//    PBBaseModel* mode{ nullptr };
//    /************/
//    QScrollArea* area{ nullptr }; /* 滚动区域 */
//};

// static QGroupBox* createResultUI(const CalibrationItem& item)
//{
//    auto parent = new QGroupBox;
//    auto widgetLayout = new QGridLayout(parent);
//    widgetLayout->setContentsMargins(0, 10, 0, 10);
//    widgetLayout->setHorizontalSpacing(10);
//    widgetLayout->setVerticalSpacing(20);

//    /* 套娃娃 在解析一次 */
//    CalibrationResult tempResult;
//    tempResult.fromByteArray(item.result);

//    /* 创建结果的参数 */
//    auto jsonResult = QJsonDocument::fromJson(tempResult.result);
//    auto jsonObject = jsonResult.object();
//    auto find = jsonObject.find("testResult");

//    if (find == jsonObject.end())
//    {
//        auto lable = new QLabel(parent);
//        lable->setText("无结果数据");
//        widgetLayout->addWidget(lable, 0, Qt::AlignCenter);
//        return parent;
//    }

//    auto jsonArray = find->toArray();
//    if (jsonArray.isEmpty())
//    {
//        auto lable = new QLabel(parent);
//        lable->setText("无结果数据");
//        widgetLayout->addWidget(lable, 0, Qt::AlignCenter);
//        return parent;
//    }

//    int i = 1;
//    for (auto iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
//    {
//        int row = 0;
//        int col = 0;
//        auto groupRes = new QGroupBox(parent);
//        groupRes->setTitle(QString("第%1组结果").arg(i));
//        auto resLayout = new QGridLayout(groupRes);
//        resLayout->setContentsMargins(0, 10, 0, 10);
//        resLayout->setHorizontalSpacing(10);
//        resLayout->setVerticalSpacing(20);

//        auto resObj = iter->toObject();
//        for (auto objIter = resObj.begin(); objIter != resObj.end(); ++objIter)
//        {
//            /* 依次是名称 数值  单位(可选) */
//            auto splitList = objIter->toString().split(" ");
//            /* 结果非法 */
//            if (splitList.size() < 2)
//            {
//                qDebug() << "当前结果非法:" << objIter.value();
//                continue;
//            }

//            auto labName = new QLabel(groupRes);
//            labName->setText(splitList.at(0) + ":");

//            auto labValue = new QLabel(groupRes);
//            if (splitList.size() == 2)
//            {
//                labValue->setText(splitList.at(1));
//            }
//            else
//            {
//                labValue->setText(splitList.at(1) + splitList.at(2));
//            }

//            resLayout->addWidget(labName, row, col++);
//            resLayout->addWidget(labValue, row, col++);

//            if (col >= 4)
//            {
//                col = 0;
//                ++row;
//            }
//        }
//        widgetLayout->addWidget(groupRes);
//        ++i;
//    }
//    return parent;
//}

// TestViewWidget::TestViewWidget(const TestAttributeInfo& info, const CalibrationItem& item, QWidget* parent)
//    : QWidget(parent)
//    , m_impl(new TestViewWidgetImpl)
//{
//    auto area = new QScrollArea(this);
//    area->setWidgetResizable(true);

//    auto widget = new QWidget;
//    auto mainLayout = new QVBoxLayout(widget);
//    mainLayout->setSpacing(30);
//    mainLayout->setContentsMargins(0, 0, 0, 0);

//    /*  创建设置的参数 */
//    auto group = initTestParaView(info.contentList, m_impl->dataWidget);
//    group->setTitle("设置参数");
//    mainLayout->addWidget(group);

//    group = createResultUI(item);
//    group->setTitle("测试结果");
//    mainLayout->addWidget(group);

//    mainLayout->addStretch();

//    area->setWidget(widget);

//    mainLayout = new QVBoxLayout(this);
//    mainLayout->setSpacing(0);
//    mainLayout->setContentsMargins(0, 0, 0, 0);
//    mainLayout->addWidget(area);
//}
// TestViewWidget::TestViewWidget(const TestAttributeInfo& info, QWidget* parent)
//    : QWidget(parent)
//    , m_impl(new TestViewWidgetImpl)
//{
//    auto area = new QScrollArea(this);
//    area->setWidgetResizable(true);

//    auto widget = new QWidget;
//    auto mainLayout = new QVBoxLayout(widget);
//    mainLayout->setSpacing(30);
//    mainLayout->setContentsMargins(0, 0, 0, 0);

//    /*  创建设置的参数 */
//    auto group = initTestParaView(info.contentList, m_impl->dataWidget);
//    group->setTitle("设置参数");
//    mainLayout->addWidget(group);

//    mainLayout->addStretch();

//    area->setWidget(widget);

//    mainLayout = new QVBoxLayout(this);
//    mainLayout->setSpacing(0);
//    mainLayout->setContentsMargins(0, 0, 0, 0);
//    mainLayout->addWidget(area);
//}

// TestViewWidget::TestViewWidget(QWidget* parent)
//    : QWidget(parent)
//    , m_impl(new TestViewWidgetImpl)
//{
//    auto layout = new QVBoxLayout(this);
//    layout->setSpacing(0);
//    layout->setContentsMargins(0, 0, 0, 0);

//    auto label = new QLabel(this);
//    label->setText("无数据");
//    label->setAlignment(Qt::AlignCenter);
//    layout->addWidget(label);
//}
// TestViewWidget::~TestViewWidget() { delete m_impl; }

// void TestViewWidget::setData(const QByteArray& array)
//{
//    auto doc = QJsonDocument::fromJson(array);
//    setData(doc.object());
//}
// void TestViewWidget::setData(const QJsonObject& jsonObject)
//{
//    auto bak = jsonObject;
//    for (auto iter = m_impl->dataWidget.begin(); iter != m_impl->dataWidget.end(); ++iter)
//    {
//        AutoTestHelper::setWidgetValue(iter.value(), iter.key(), bak);
//    }
//}
