#include "TaskReportTableWidget.h"
#include "ui_TaskReportTableWidget.h"

#include "GlobalData.h"
#include "PageNavigator.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskReportMessageSerialize.h"
#include "TaskReportTableModel.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextBlock>
#include <QTextStream>

TaskReportTableWidget::TaskReportTableWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskReportTableWidget)
    , d_pageNavigator(nullptr)
{
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

TaskReportTableWidget::~TaskReportTableWidget() { delete ui; }

void TaskReportTableWidget::currentPageChanged(int page)
{
    SERVICEONLINECHECK();

    if (ui->startDate->date() > ui->endDate->date())
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    DeviceWorkTaskConditionQuery condition;
    condition.taskStatus = ui->taskStatusComboBox->currentData().toInt();
    condition.beginDate = ui->startDate->dateTime();
    condition.endDate = ui->endDate->dateTime();
    condition.totalCount = 0;
    condition.currentPage = page;
    condition.pageSize = ui->pagesize->currentData().toInt();

    QByteArray data;
    data << condition;
    emit signalGetHistoryPlan(data);
}

void TaskReportTableWidget::init()
{
    if (nullptr == d_pageNavigator)
    {
        d_pageNavigator = new PageNavigator();
        ui->nav_bar_layout->addWidget(d_pageNavigator);
        connect(d_pageNavigator, SIGNAL(currentPageChanged(int)), this, SLOT(currentPageChanged(int)));
    }
    QDateTime currentTime = GlobalData::currentDateTime();
    ui->startDate->setDateTime(currentTime.addMonths(-1));
    ui->endDate->setDateTime(currentTime);

    mTaskReportTableModel = new TaskReportTableModel(this);
    ui->taskPlanTableView->setModel(mTaskReportTableModel);
    // 设置表格样式
    ui->taskPlanTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->taskPlanTableView->horizontalHeader()->setStretchLastSection(true);
    ui->taskPlanTableView->horizontalHeader()->setHighlightSections(false);
    ui->taskPlanTableView->verticalHeader()->setVisible(false);
    ui->taskPlanTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->taskPlanTableView->setFrameShape(QFrame::NoFrame);
    ui->taskPlanTableView->setAlternatingRowColors(true);

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

    connect(ui->exportBtn, &QPushButton::clicked, this, &TaskReportTableWidget::slotExportBtnClicked);
    connect(ui->selectBtn, &QPushButton::clicked, this, &TaskReportTableWidget::slotQueryBtnClicked);

    m_initFinish = true;
}

void TaskReportTableWidget::slotGetHistoryPlan(const QByteArray& data)
{
    auto ackResult = Optional<DeviceWorkTaskConditionQueryACK>::emptyOptional();
    QByteArray bak = data;
    bak >> ackResult;

    DeviceWorkTaskConditionQueryACK ack;
    if (ackResult)
    {
        ack = ackResult.value();
    }

    mTaskReportTableModel->setDeviceWorkTaskDatas(ack.deviceWorkTasks);
    int pageSize = ui->pagesize->currentData().toInt();
    //查询到的分页数
    int pageNum = ack.totalCount / pageSize + ((ack.totalCount % pageSize) ? 1 : 0);
    d_pageNavigator->setMaxPage(pageNum);
    d_pageNavigator->setCurrentPage(ack.currentPage);
}

void TaskReportTableWidget::slotGetDataByUUID(const QByteArray& data)
{
    QList<TaskReportTable> dataList;
    QByteArray bak = data;
    bak >> dataList;

    for (auto& data : dataList)
    {
        QString createTimeStr = QString("计划创建时间：");
        QString preStartTimeStr = QString("准备开始时间：");
        QString startTimeStr = QString("任务开始时间：");
        QString endTimeStr = QString("计划结束时间：");
        QString taskCodeStr = QString("任务代号：");

        QString createTime = data.createTime;
        QString preStartTime = data.preStartTime;
        QString startTime = data.startTime;
        QString endTime = data.endTime;
        QString taskCode = data.taskCode;

        createTime.replace("T", " ");
        preStartTime.replace("T", " ");
        startTime.replace("T", " ");
        endTime.replace("T", " ");

        createTime.replace(".000", "");
        preStartTime.replace(".000", "");
        startTime.replace(".000", "");
        endTime.replace(".000", "");

        ui->plainTextEdit->appendPlainText(taskCodeStr + taskCode + "\n" + createTimeStr + createTime + "\n" + preStartTimeStr + preStartTime + "\n" +
                                           startTimeStr + startTime + "\n" + endTimeStr + endTime);
    }
}

void TaskReportTableWidget::slotExportBtnClicked()
{
    auto fileName = QFileDialog::getSaveFileName(this, QString("导出任务报表"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            return;
        }
        QTextStream stream(&file);
        QTextDocument* document = ui->plainTextEdit->document();
        QTextBlock textBlock;
        for (textBlock = document->begin(); textBlock != document->end(); textBlock = textBlock.next())
        {
            stream << textBlock.text().toUtf8() << '\n';
        }
        file.close();
    }
}

void TaskReportTableWidget::slotQueryBtnClicked()
{
    SERVICEONLINECHECK();

    if (ui->startDate->dateTime() > ui->endDate->dateTime())
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    DeviceWorkTaskConditionQuery condition;
    condition.taskStatus = ui->taskStatusComboBox->currentData().toInt();
    condition.beginDate = ui->startDate->dateTime();
    condition.endDate = ui->endDate->dateTime();
    condition.totalCount = 0;
    condition.currentPage = 1;
    condition.pageSize = ui->pagesize->currentData().toInt();

    QByteArray data;
    data << condition;
    emit signalGetHistoryPlan(data);
}

void TaskReportTableWidget::on_pagesize_currentIndexChanged(const QString& arg1)
{
    Q_UNUSED(arg1);
    if (m_initFinish)
        slotQueryBtnClicked();
}

void TaskReportTableWidget::on_taskStatusComboBox_currentIndexChanged(const QString& arg1)
{
    Q_UNUSED(arg1);
    if (m_initFinish)
        slotQueryBtnClicked();
}

void TaskReportTableWidget::on_taskPlanTableView_clicked(const QModelIndex& index)
{
    ui->plainTextEdit->clear();
    if (!index.isValid())
    {
        return;
    }
    int row = index.row();
    if (row != -1)
    {
        auto deviceWorkTaskModel = mTaskReportTableModel->getDeviceWorkTaskData(index).value();
        auto uuid = deviceWorkTaskModel.m_uuid;
        JsonWriter writer;
        writer& uuid;
        emit signalGetDataByUUID(writer.GetByteArray());
    }
}
