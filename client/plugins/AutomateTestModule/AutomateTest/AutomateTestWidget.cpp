#include "AutomateTestWidget.h"
#include "AutomateTestConfig.h"
#include "AutomateTestModel.h"
#include "AutomateTestResultDialog.h"
#include "AutomateTestSerialize.h"
#include "AutomateTestUtility.h"
#include "CppMicroServicesUtility.h"
#include "CreateTestPlanWidget.h"
#include "GlobalData.h"
#include "ParameterBindingConfig.h"
#include "ParameterBindingSelect.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "TestAttributeWidget.h"
#include "SubscriberHelper.h"
#include "ui_AutomateTestWidget.h"
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTableView>
#include <QUuid>

class AutomateTestWidgetImpl
{
public:
    AutomateTestItemInfo testItemInfo; /* 从配置文件读取的支持测试的模式 */
    AutomateTestModel model;           /* 显示已经添加的测试项的数据 */
    TestParameterWidget* testParameterWidget{ nullptr };
    QPushButton* curProgressButton{ nullptr };
    QMap<QString, QTabWidget*> attrTabWidgetMap;
    cppmicroservices::BundleContext context;
    cpp_redis::subscriber logSuber;

    static AutomateTestPlanItemList getAllData()
    {
        /* 从Redis获取数据 */
        AutomateTestPlanItemList automateTestPlanItemList;
        GlobalData::getAutomateTestPlanItemList(automateTestPlanItemList);
        return automateTestPlanItemList;
    }
    static void changeButtonToGreen(QPushButton* button)
    {
        static constexpr const char* colorQss = "QPushButton{background-color: rgb(0, 255, 0);color: rgb(0, 0, 0);}";
        if (button == nullptr)
        {
            return;
        }
        button->setStyleSheet(colorQss);
    }
    static void changeButtonToRed(QPushButton* button)
    {
        static constexpr const char* colorQss = "QPushButton{background-color: rgb(255, 0, 0);color: rgb(0, 0, 0);}";
        if (button == nullptr)
        {
            return;
        }
        button->setStyleSheet(colorQss);
    }
    static void changeButtonToGrey(QPushButton* button)
    {
        static constexpr const char* colorQss = "QPushButton{background-color: rgb(125, 125, 125);color: rgb(0, 0, 0);}";
        if (button == nullptr)
        {
            return;
        }
        button->setStyleSheet(colorQss);
    }
};

AutomateTestWidget::AutomateTestWidget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AutomateTestWidget)
    , m_impl(new AutomateTestWidgetImpl)
{
    ui->setupUi(this);
    m_impl->context = context;

    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->model));

    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    connect(ui->tableView, &QAbstractItemView::clicked, this, &AutomateTestWidget::showTreeItem);
    connect(ui->addTestProject, &QAbstractButton::clicked, this, &AutomateTestWidget::showAddTestItemWidget);
    connect(ui->delTestProject, &QAbstractButton::clicked, this, &AutomateTestWidget::deleteTestItem);
    connect(ui->parameterBinding, &QAbstractButton::clicked, this, &AutomateTestWidget::showParameterBindingWidget);
    connect(ui->resultQuery, &QAbstractButton::clicked, this, &AutomateTestWidget::showTestResultWidget);
    connect(ui->beginTest, &QAbstractButton::clicked, this, &AutomateTestWidget::startTest);
    connect(ui->endTest, &QAbstractButton::clicked, this, &AutomateTestWidget::stopTest);
    connect(this, &AutomateTestWidget::sg_recvLogs, this, &AutomateTestWidget::recvLogs);
    /* 测试使用 */
    connect(ui->endOfTest, &QAbstractButton::clicked, this, &AutomateTestWidget::testRefreshConfig);

    /* 重置进度条 */
    resetProgressBar();
    /* 解析配置文件 */
    auto automateTestConfig = AutomateTestConfig::parseTestItemConfig();
    if (automateTestConfig)
    {
        m_impl->testItemInfo = automateTestConfig.value();
    }
    else
    {
        qWarning() << automateTestConfig.msg();
    }

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_statusChange,this,&AutomateTestWidget::masterStatusChange);
}

AutomateTestWidget::~AutomateTestWidget()
{
    delete ui;
    delete m_impl;
}
void AutomateTestWidget::refreshData()
{
    auto data = AutomateTestWidgetImpl::getAllData();
    m_impl->model.setTestItemData(data);
}

void AutomateTestWidget::deleteTestItemACK(const QByteArray& data)
{
    auto ack = Optional<AutomateTestPlanItem>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, "提示", "删除失败", "确认");
    }
    refreshData();
}
void AutomateTestWidget::startTestACK(const QByteArray& data)
{
    auto bak = data;
    TestProcessACK ack;
    bak >> ack;
    switch (ack.status)
    {
    case AutomateTestStatus::Failed:
    {
        AutomateTestWidgetImpl::changeButtonToRed(m_impl->curProgressButton);
        ui->beginTest->setEnabled(true);
        ui->endTest->setEnabled(false);
        unregisterSuber();
        // QMessageBox::critical(this, "测试错误", ack.msg, "确认");
        break;
    }
    case AutomateTestStatus::Busy:
    {
        ui->beginTest->setEnabled(true);
        ui->endTest->setEnabled(false);
        unregisterSuber();
        recvLogs(ack.msg);
        QMessageBox::critical(this, "提示", ack.msg, "确认");
        break;
    }
    case AutomateTestStatus::LinkConfiguration:
    {
        m_impl->curProgressButton = ui->linkConfiguration;
        AutomateTestWidgetImpl::changeButtonToGreen(ui->linkConfiguration);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->parameterSetting);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->startTesting);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->testing);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->endOfTest);
        break;
    }
    case AutomateTestStatus::ParameterSetting:
    {
        m_impl->curProgressButton = ui->parameterSetting;

        AutomateTestWidgetImpl::changeButtonToGreen(ui->linkConfiguration);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->parameterSetting);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->startTesting);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->testing);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->endOfTest);
        break;
    }
    case AutomateTestStatus::StartTesting:
    {
        m_impl->curProgressButton = ui->startTesting;

        AutomateTestWidgetImpl::changeButtonToGreen(ui->linkConfiguration);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->parameterSetting);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->startTesting);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->testing);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->endOfTest);
        break;
    }
    case AutomateTestStatus::Testing:
    {
        m_impl->curProgressButton = ui->testing;

        AutomateTestWidgetImpl::changeButtonToGreen(ui->linkConfiguration);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->parameterSetting);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->startTesting);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->testing);
        AutomateTestWidgetImpl::changeButtonToGrey(ui->endOfTest);
        break;
    }
    case AutomateTestStatus::EndOfTest:
    {
        ui->beginTest->setEnabled(true);
        ui->endTest->setEnabled(false);
        m_impl->curProgressButton = ui->endOfTest;

        AutomateTestWidgetImpl::changeButtonToGreen(ui->linkConfiguration);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->parameterSetting);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->startTesting);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->testing);
        AutomateTestWidgetImpl::changeButtonToGreen(ui->endOfTest);
        unregisterSuber();
        break;
    }
    }
}
void AutomateTestWidget::stopTestACK(const QByteArray& /*data*/) {}
void AutomateTestWidget::queryTestItemACK(const QByteArray& /*data*/) {}

void AutomateTestWidget::showTestResultWidget()
{
    AutomateTestResultDialog resultDialog(this);
    resultDialog.intiUIData(m_impl->testItemInfo);
    resultDialog.exec();
}

void AutomateTestWidget::showParameterBindingWidget()
{
    ParameterBindingSelect parameterBindingSelect(m_impl->testItemInfo, this);
    auto res = parameterBindingSelect.exec();
    parameterBindingSelect.setParent(nullptr);

    if (QDialog::Accepted != res)
    {
        return;
    }

    auto id = parameterBindingSelect.id();
    auto find = m_impl->testItemInfo.itemMap.find(id);
    if (find == m_impl->testItemInfo.itemMap.end())
    {
        QMessageBox::warning(this, "提示", "未找到当前测试项的装订参数", "确定");
        return;
    }
    ParameterBindingConfig parameterBindingConfig(this);
    auto result = parameterBindingConfig.init(find.value());
    if (!result)
    {
        QMessageBox::warning(this, "查找装订参数错误", result.msg(), "确定");
        return;
    }
    connect(this, &AutomateTestWidget::sg_addTestBindItemACK, &parameterBindingConfig, &ParameterBindingConfig::addTestBindItemACK);
    connect(this, &AutomateTestWidget::sg_deleteTestBindItemACK, &parameterBindingConfig, &ParameterBindingConfig::deleteTestBindItemACK);
    parameterBindingConfig.exec();
}

void AutomateTestWidget::showAddTestItemWidget()
{
    CreateTestPlanWidget dialog(this);
    dialog.init(m_impl->testItemInfo);
    dialog.setWindowTitle("新增测试计划");
    auto retsult = dialog.exec();
    dialog.setParent(nullptr);
    if (QDialog::Accepted == retsult)
    {
        refreshData();
        return;
    }
}

void AutomateTestWidget::deleteTestItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AutomateTestPlanItem automateTestPlanItem;
    if (!m_impl->model.testItem(ui->tableView->currentIndex(), automateTestPlanItem))
    {
        QMessageBox::critical(this, QString("提示"), "请选择要删除的项", QString("确定"));
        return;
    }
    QByteArray tempData;
    tempData << automateTestPlanItem;
    emit sg_deleteTestItem(tempData);
}

/* 开始测试 */
void AutomateTestWidget::startTest()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AutomateTestPlanItem item;
    if (!m_impl->model.testItem(ui->tableView->currentIndex(), item))
    {
        QMessageBox::critical(this, QString("提示"), "请选择要进行测试的项目", QString("确定"));
        return;
    }
    registerSuber();
    ui->beginTest->setEnabled(false);
    ui->endTest->setEnabled(true);
    ui->statusInfo->clear();
    resetProgressBar();

    QByteArray array;
    array << item;
    emit sg_startTest(array);
}

/* 结束测试 */
void AutomateTestWidget::stopTest()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    ui->beginTest->setEnabled(true);
    ui->endTest->setEnabled(false);
    resetProgressBar();
    unregisterSuber();
    emit sg_stopTest(QByteArray());
}

void AutomateTestWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    refreshData();
}
void AutomateTestWidget::showTreeItem(const QModelIndex& index)
{
    AutomateTestPlanItem item;
    if (!m_impl->model.testItem(index, item))
    {
        return;
    }
    auto automateTestItem = m_impl->testItemInfo.itemMap.value(item.testTypeID);

    if (m_impl->testParameterWidget != nullptr)
    {
        ui->stackedWidget->removeWidget(m_impl->testParameterWidget);
        m_impl->testParameterWidget->setParent(nullptr);
        delete m_impl->testParameterWidget;
    }

    m_impl->testParameterWidget = new TestParameterWidget(this);
    m_impl->testParameterWidget->setEnabled(false);
    m_impl->testParameterWidget->init(automateTestItem, item.workMode);
    m_impl->testParameterWidget->setData(item);
    ui->stackedWidget->addWidget(m_impl->testParameterWidget);
    ui->stackedWidget->setCurrentWidget(m_impl->testParameterWidget);
}

void AutomateTestWidget::appendMessage(const QString& text) { ui->statusInfo->append(text); }
void AutomateTestWidget::clearMessage() { ui->statusInfo->clear(); }
void AutomateTestWidget::resetProgressBar()
{
    AutomateTestWidgetImpl::changeButtonToGrey(ui->linkConfiguration);
    AutomateTestWidgetImpl::changeButtonToGrey(ui->parameterSetting);
    AutomateTestWidgetImpl::changeButtonToGrey(ui->startTesting);
    AutomateTestWidgetImpl::changeButtonToGrey(ui->testing);
    AutomateTestWidgetImpl::changeButtonToGrey(ui->endOfTest);
    m_impl->curProgressButton = nullptr;
}
void AutomateTestWidget::recvLogs(const QString& syslog) { ui->statusInfo->append(syslog); }
void AutomateTestWidget::unregisterSuber() { m_impl->logSuber.disconnect(); }

void AutomateTestWidget::masterStatusChange()
{
    if (m_impl->logSuber.is_connected())
    {
        m_impl->logSuber.disconnect();
        registerSuber();
    }
}
void AutomateTestWidget::registerSuber()
{
    if (m_impl->logSuber.is_connected())
    {
        return;
    }
    if (!RedisHelper::getInstance().getSubscriber(m_impl->logSuber))
    {
        return;
    }
    m_impl->logSuber.subscribe("AutomateTest", [=](const std::string& /*chanel*/, const std::string& msg) {
        QString qmsg = QString::fromStdString(msg);
        emit sg_recvLogs(qmsg);
    });
    m_impl->logSuber.commit();
}

void AutomateTestWidget::testRefreshConfig()
{
    //    m_impl->paramMap.clear();
    //    m_impl->uiParamMap.clear();
    //    AutoTestHelper::parseTreeConfig(m_impl->paramMap, m_impl->uiParamMap);
}
