#include "AutomateTestResultDialog.h"
#include "AutomateTestConfig.h"
#include "AutomateTestResultModel.h"
#include "AutomateTestSerialize.h"
#include "AutomateTestWidget.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"
#include "TestAttributeWidget.h"
#include "ui_AutomateTestResultDialog.h"
#include <QMessageBox>
#include <QScrollArea>

class AutomateTestResultDialogImpl
{
public:
    AutomateTestItemInfo testItemInfo; /* 从配置文件读取的支持测试的模式 */
    AutomateTestResultModel model;
    AutomateTestWidget* parent{ nullptr };
    // TestParameterWidget* paramWidget{ nullptr };
    // TestSystemParamMap* configMap;

    AutomateTestResultDialogImpl(AutomateTestWidget* parent_)
        : parent(parent_)

    {
    }
};

AutomateTestResultDialog::AutomateTestResultDialog(AutomateTestWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AutomateTestResultDialog)
    , m_impl(new AutomateTestResultDialogImpl(parent))
{
    Q_ASSERT(parent != nullptr);
    ui->setupUi(this);

    ui->tableView->setModel(&(m_impl->model));
    Utility::tableViewInit(ui->tableView, true);

    auto curDate = GlobalData::currentDate();
    ui->beginDate->setDate(curDate.addDays(-7));
    ui->endDate->setDate(curDate);

    //    ui->resultPage->setMinimumWidth(700);

    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    connect(ui->tableView, &QTableView::clicked, this, &AutomateTestResultDialog::tableViewClicked);
    connect(ui->search, &QAbstractButton::clicked, this, &AutomateTestResultDialog::selectItem);
    connect(ui->finishing, &QAbstractButton::clicked, this, &AutomateTestResultDialog::finishingItem);
    connect(ui->remove, &QAbstractButton::clicked, this, &AutomateTestResultDialog::deleteItem);

    connect(parent, &AutomateTestWidget::sg_clearHistryACK, this, &AutomateTestResultDialog::finishingItemACK);
    connect(parent, &AutomateTestWidget::sg_deleteTestResultACK, this, &AutomateTestResultDialog::deleteItemACK);
    connect(parent, &AutomateTestWidget::sg_queryTestResultACK, this, &AutomateTestResultDialog::selectItemACK);

    /* 测试数据 */
    //    AutomateTestResultGroup group;
    //    group.groupName = "测试测试测试";

    //    for (int i = 0; i < 5; ++i)
    //    {
    //        AutomateTestItemParameterInfo info;
    //        info.id = "id";
    //        info.rawValue = "当前项的原始值";
    //        info.displayName = "当前项的显示名称";
    //        info.displayValue = "当前项的显示值";
    //        group.resultList << info;
    //    }
    //    AutomateTestResultGroupList tempList;
    //    for (int i = 0; i < 5; ++i)
    //    {
    //        tempList << group;
    //    }

    //    AutomateTestResult result;
    //    result.id = "id";
    //    result.testTypeID = "1_GT";
    //    result.testTypeName = "G/T值测试";
    //    result.testPlanName = "对应创建测试项的名称";
    //    result.testResultList = tempList;
    //    m_impl->model.updateData({ result });
}

AutomateTestResultDialog::~AutomateTestResultDialog()
{
    delete ui;
    delete m_impl;
}
void AutomateTestResultDialog::intiUIData(const AutomateTestItemInfo& info)
{
    ui->testType->clear();
    ui->testType->addItem("所有");
    m_impl->testItemInfo = info;
    for (auto& group : info.groupList)
    {
        for (auto& item : group.itemList)
        {
            ui->testType->addItem(item.name, item.id);
        }
    }
}
/* 查询所有的记录 */
void AutomateTestResultDialog::selectItem()
{
    SERVICEONLINECHECK();

    AutomateTestResultQuery query;
    query.beginTime = ui->beginDate->dateTime();
    query.endTime = ui->endDate->dateTime();
    query.testTypeID = ui->testType->currentData().toString();
    if (query.beginTime > query.endTime)
    {
        QMessageBox::warning(this, "提示", "查询开始时间能晚于结束时间", QString("确定"));
        return;
    }

    m_impl->model.clear();

    QByteArray array;
    array << query;
    emit m_impl->parent->sg_queryTestResult(array);
}
void AutomateTestResultDialog::selectItemACK(const QByteArray& value)
{
    auto result = Optional<AutomateTestResultList>::emptyOptional();
    auto bak = value;
    bak >> result;

    if (!result)
    {
        QMessageBox::warning(this, "查询数据失败", result.msg(), QString("确定"));
        return;
    }

    m_impl->model.updateData(result.value());
}

/* 删除一条记录 */
void AutomateTestResultDialog::deleteItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto result = m_impl->model.getItem(ui->tableView->currentIndex());
    if (!result)
    {
        QMessageBox::warning(this, QString("提示"), "请选择要删除的项", QString("确定"));
        return;
    }

    QByteArray array;
    array << result.value();
    emit m_impl->parent->sg_deleteTestResult(array);
}
void AutomateTestResultDialog::deleteItemACK(const QByteArray& value)
{
    auto result = Optional<AutomateTestResult>::emptyOptional();
    auto bak = value;
    bak >> result;

    if (!result)
    {
        QMessageBox::warning(this, "删除数据失败", result.msg(), QString("确定"));
        return;
    }
    m_impl->model.removeValue(result.value());
    QMessageBox::warning(this, "提示", "删除成功", QString("确定"));
}

void AutomateTestResultDialog::finishingItem()
{
    SERVICEONLINECHECK();
    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (1 == QMessageBox::warning(this, "是否继续?", "此操作会清空90天之前的数据,不可恢复", "继续", "取消"))
    {
        return;
    }

    ClearAutomateTestHistry item;
    QByteArray array;
    array << item;
    emit m_impl->parent->sg_clearHistry(array);
}
void AutomateTestResultDialog::finishingItemACK(const QByteArray& value)
{
    auto result = OptionalNotValue::emptyOptional();
    auto bak = value;
    bak >> result;

    if (!result)
    {
        QMessageBox::warning(this, "清理数据失败", result.msg(), QString("确定"));
        return;
    }
    QMessageBox::warning(this, "提示", "清理数据成功", QString("确定"));
}

void AutomateTestResultDialog::tableViewClicked(const QModelIndex& index)
{
    auto result = m_impl->model.getItem(index);
    if (!result)
    {
        return;
    }
    auto& testResult = result.value();

    /* 先清空当前的界面 */
    auto count = ui->resultPage->count();
    for (int i = 0; i < count; ++i)
    {
        auto widget = ui->resultPage->widget(i);
        ui->resultPage->removeTab(i);
        if (widget != nullptr)
        {
            delete widget;
        }
    }
    ui->resultPage->clear();

    /* 测试参数页面 */
    auto automateTestItem = m_impl->testItemInfo.itemMap.value(testResult.testTypeID);
    auto paramWidget = new TestParameterWidget(this);
    paramWidget->setEnabled(false);
    paramWidget->init(automateTestItem, testResult.testParam.workMode);
    paramWidget->setData(testResult.testParam);
    ui->resultPage->addTab(paramWidget, "测试参数");

    /* 测试结果页面 */
    /* 20210701wp?? 我没见过测试结果长什么样  这些全是我全凭想象弄出来的 */
    auto tempResultPage = new AutomateTestResultWidgetImpl;
    tempResultPage->initUI(testResult.testResultList);
    ui->resultPage->addTab(tempResultPage, "测试结果");
}

/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/

AutomateTestResultWidgetImpl::AutomateTestResultWidgetImpl(QWidget* parent)
    : QWidget(parent)
{
}
AutomateTestResultWidgetImpl::~AutomateTestResultWidgetImpl() {}
void AutomateTestResultWidgetImpl::initUI(const AutomateTestResultGroupList& resultList)
{
    if (resultList.isEmpty())
    {
        initEmptyUI("当前测试项无结果数据展示");
        return;
    }

    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    auto widget = new QWidget(scrollArea);
    auto layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(0, 10, 0, 0);

    CommonToWidgetInfo tempToWidgetInfo;
    tempToWidgetInfo.important = false;
    tempToWidgetInfo.type = DynamicConfigToWidgetType::Label;

    for (auto& item : resultList)
    {
        int row = 0;
        int col = 0;

        auto groupBox = new QGroupBox(widget);
        auto widgetLayout = new QGridLayout(groupBox);

        groupBox->setTitle(item.groupName);
        widgetLayout->setContentsMargins(0, 15, 0, 10);
        widgetLayout->setHorizontalSpacing(10);
        widgetLayout->setVerticalSpacing(20);

        for (auto& resultItem : item.resultList)
        {
            tempToWidgetInfo.id = resultItem.id;
            tempToWidgetInfo.name = resultItem.displayName;

            LabelToWidgetInfo info;
            info.initValue = resultItem.displayValue;
            tempToWidgetInfo.toWidgetInfo = QVariant::fromValue<LabelToWidgetInfo>(info);

            auto widgetResult = DynamicConfigToWidgetUtility::createWidget(tempToWidgetInfo);
            if (!widgetResult || widgetResult->widget == nullptr)
            {
                continue;
            }
            auto& dynamicWidgetInfo = widgetResult.value();
            dynamicWidgetInfo.widget->setParent(groupBox);

            auto label = new QLabel(widget);
            label->setText(resultItem.displayName + ":");

            label->setFixedWidth(150);
            dynamicWidgetInfo.widget->setFixedWidth(200);

            widgetLayout->addWidget(label, row, col++);
            widgetLayout->addWidget(widgetResult->widget, row, col++);
            if (col >= 4)
            {
                col = 0;
                ++row;
            }
        }
        layout->addWidget(groupBox);
    }

    scrollArea->setWidget(widget);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
}
void AutomateTestResultWidgetImpl::initEmptyUI(const QString& tips)
{
    auto layout = new QHBoxLayout(this);
    auto tipsLabel = new QLabel(this);
    tipsLabel->setText(tips);
    tipsLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(tipsLabel);
}
