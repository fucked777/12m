#include "TaskExecuteLogWidget.h"
#include "ui_TaskExecuteLogWidget.h"

#include "AutoRunTaskLogMessageSerialize.h"
#include "AutoTaskLogTableModel.h"
#include "PageNavigator.h"
#include "QssCommonHelper.h"
//#include "ServiceCheck.h"
#include "TaskPlanMessageDefine.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskPlanTableModel.h"

#include <QMessageBox>

TaskExecuteLogWidget::TaskExecuteLogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskExecuteLogWidget)
    , d_pageNavigator(nullptr)
{
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "common.qss");

    init();
}

TaskExecuteLogWidget::~TaskExecuteLogWidget() { delete ui; }

void TaskExecuteLogWidget::currentPageChanged(int page)
{
    //    SERVICEONLINECHECK();

    if (ui->startTime->date() > ui->endTime->date())
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    DeviceWorkTaskConditionQuery condition;
    condition.taskStatus = ui->taskStatusComboBox->currentData().toInt();
    condition.beginDate = ui->startTime->dateTime();
    condition.endDate = ui->endTime->dateTime();
    condition.totalCount = 0;
    condition.currentPage = page;
    condition.pageSize = ui->pagesize->currentData().toInt();

    QByteArray data;
    data << condition;
    emit signalGetHistoryPlan(data);
}

void TaskExecuteLogWidget::init()
{
    if (nullptr == d_pageNavigator)
    {
        d_pageNavigator = new PageNavigator();
        ui->nav_bar_layout->addWidget(d_pageNavigator);
        connect(d_pageNavigator, SIGNAL(currentPageChanged(int)), this, SLOT(currentPageChanged(int)));
    }
    QDateTime currentTime = GlobalData::currentDateTime();
    ui->startTime->setDateTime(currentTime.addMonths(-1));
    ui->endTime->setDateTime(currentTime);

    mTaskPlanTableModel = new TaskPlanTableModel(this);
    ui->taskPlanTableView->setModel(mTaskPlanTableModel);
    // 设置表格样式
    ui->taskPlanTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->taskPlanTableView->horizontalHeader()->setStretchLastSection(true);
    ui->taskPlanTableView->horizontalHeader()->setHighlightSections(false);
    ui->taskPlanTableView->verticalHeader()->setVisible(false);
    ui->taskPlanTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->taskPlanTableView->setFrameShape(QFrame::NoFrame);
    ui->taskPlanTableView->setAlternatingRowColors(true);

    mAutoTaskLogTableModel = new AutoTaskLogTableModel(this);
    ui->autoTaskLogTableView->setModel(mAutoTaskLogTableModel);
    // 设置表格样式
    ui->autoTaskLogTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->autoTaskLogTableView->horizontalHeader()->setStretchLastSection(true);
    ui->autoTaskLogTableView->horizontalHeader()->setHighlightSections(false);
    ui->autoTaskLogTableView->verticalHeader()->setVisible(false);
    ui->autoTaskLogTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->autoTaskLogTableView->setFrameShape(QFrame::NoFrame);
    ui->autoTaskLogTableView->setAlternatingRowColors(true);

    // 设置分页大小
    ui->pagesize->addItem("10", 10);
    ui->pagesize->addItem("25", 25);
    ui->pagesize->addItem("50", 50);
    ui->pagesize->addItem("100", 100);

    // 设置任务状态
    ui->taskStatusComboBox->addItem(QString("全部"), static_cast<int>(TaskPlanStatus::NoStart) - 1);
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::NoStart), static_cast<int>(TaskPlanStatus::NoStart));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::Running), static_cast<int>(TaskPlanStatus::Running));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::TimeOut), static_cast<int>(TaskPlanStatus::TimeOut));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::Warning), static_cast<int>(TaskPlanStatus::Warning));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::Finish), static_cast<int>(TaskPlanStatus::Finish));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::Error), static_cast<int>(TaskPlanStatus::Error));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::Exception), static_cast<int>(TaskPlanStatus::Exception));
    ui->taskStatusComboBox->addItem(TaskPlanHelper::taskStatusToString(TaskPlanStatus::CTNoStart), static_cast<int>(TaskPlanStatus::CTNoStart));

    connect(ui->selectBtn, &QPushButton::clicked, this, &TaskExecuteLogWidget::slotQueryBtnClicked);

    m_initFinish = true;

    ui->taskPlanTableView->setColumnWidth(0, 160);
    ui->taskPlanTableView->setColumnWidth(1, 200);
    ui->taskPlanTableView->setColumnWidth(2, 200);
    ui->taskPlanTableView->setColumnWidth(3, 200);

    ui->autoTaskLogTableView->setColumnWidth(0, 240);
    ui->autoTaskLogTableView->setColumnWidth(1, 80);
    ui->autoTaskLogTableView->setColumnWidth(2, 60);
    ui->autoTaskLogTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void TaskExecuteLogWidget::slotGetHistoryPlan(const QByteArray& data)
{
    auto ackResult = Optional<DeviceWorkTaskConditionQueryACK>::emptyOptional();
    QByteArray bak = data;
    bak >> ackResult;

    DeviceWorkTaskConditionQueryACK ack;
    if (ackResult)
    {
        ack = ackResult.value();
    }

    mTaskPlanTableModel->setDeviceWorkTaskDatas(ack.deviceWorkTasks);
    int pageSize = ui->pagesize->currentData().toInt();
    //查询到的分页数
    int pageNum = ack.totalCount / pageSize + ((ack.totalCount % pageSize) ? 1 : 0);
    d_pageNavigator->setMaxPage(pageNum);
    d_pageNavigator->setCurrentPage(ack.currentPage);
}

void TaskExecuteLogWidget::slotQueryAutoTaskLog(const QByteArray& data)
{
    auto ack = Optional<AutoTaskLogDataList>::emptyOptional();
    data >> ack;

    if (ack.success())
    {
        mAutoTaskLogTableModel->setAutoTaskLogDatas(ack.value());
    }
}

void TaskExecuteLogWidget::slotQueryBtnClicked()
{
    //    SERVICEONLINECHECK();

    if (ui->startTime->dateTime() > ui->endTime->dateTime())
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    DeviceWorkTaskConditionQuery condition;
    condition.taskStatus = ui->taskStatusComboBox->currentData().toInt();
    condition.beginDate = ui->startTime->dateTime();
    condition.endDate = ui->endTime->dateTime();
    condition.totalCount = 0;
    condition.currentPage = 1;
    condition.pageSize = ui->pagesize->currentData().toInt();

    QByteArray data;
    data << condition;
    emit signalGetHistoryPlan(data);
}

void TaskExecuteLogWidget::on_pagesize_currentIndexChanged(const QString& arg1)
{
    Q_UNUSED(arg1);
    if (m_initFinish)
        slotQueryBtnClicked();
}

void TaskExecuteLogWidget::on_taskStatusComboBox_currentIndexChanged(const QString& arg1)
{
    Q_UNUSED(arg1);
    if (m_initFinish)
        slotQueryBtnClicked();
}

void TaskExecuteLogWidget::on_taskPlanTableView_clicked(const QModelIndex& index)
{
    //    SERVICEONLINECHECK();

    if (!index.isValid())
    {
        return;
    }
    int row = index.row();
    if (row != -1)
    {
        auto deviceWorkTaskData = mTaskPlanTableModel->getDeviceWorkTaskData(index).value();
        AutoTaskLogDataCondition condition;
        condition.uuid = deviceWorkTaskData.m_uuid;
        //        condition.startTime = deviceWorkTaskData.m_startTime.toString(DATETIME_ORIGIN_FORMAT).toLongLong();
        condition.startTime = deviceWorkTaskData.m_preStartTime.toString(DATETIME_ORIGIN_FORMAT).toLongLong();
        condition.endTime = deviceWorkTaskData.m_endTime.toString(DATETIME_ORIGIN_FORMAT).toLongLong();

        QByteArray data;
        data << condition;
        emit signalQueryAutoTaskLog(data);
    }
}
