#include "SystemLogWidget.h"
#include "ui_SystemLogWidget.h"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "PageNavigator.h"
#include "QssCommonHelper.h"
#include "SystemLogMessageSerialize.h"
#include "SystemLogTableModel.h"

SystemLogWidget::SystemLogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemLogWidget)
{
    ui->setupUi(this);

    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();

    startTimer(1000);
}

SystemLogWidget::~SystemLogWidget() { delete ui; }

void SystemLogWidget::slotLogConditionQuery(const QByteArray& dataArray)
{
    QByteArray tempArray = dataArray;

    SystemLogACK ack;
    tempArray >> ack;

    int totalCount = ack.totalCount;
    if (totalCount < 0)
    {
        return;
    }
    auto pageSize = ui->pagesize->currentData().toInt();

    int pageNum = totalCount / pageSize + ((totalCount % pageSize) ? 1 : 0);
    mSystemLogTableModel->setSystemLogData(ack.dataList);
    d_pageNavigator->setMaxPage(pageNum);
    d_pageNavigator->setCurrentPage(ack.currentPage);
}

void SystemLogWidget::logLevelChange(const QString& arg1)
{
    Q_UNUSED(arg1)
    queryLogs();
}

void SystemLogWidget::pageSizeChange(const QString& arg1)
{
    Q_UNUSED(arg1)
    queryLogs();
}

void SystemLogWidget::init()
{
    QDateTime currentTime = GlobalData::currentDateTime();
    ui->startTime->setDateTime(currentTime.addMonths(-1));
    ui->endTime->setDateTime(currentTime);

    mSystemLogTableModel = new SystemLogTableModel(this);
    ui->tableView->setModel(mSystemLogTableModel);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->setWordWrap(true);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setAlternatingRowColors(true);

    // 设置分页大小
    ui->pagesize->addItem("10", 10);
    ui->pagesize->addItem("50", 50);
    ui->pagesize->addItem("100", 100);

    // 设置日志等级
    ui->logLevel->addItem("全部", static_cast<int>(LogLevel::Info) - 1);
    ui->logLevel->addItem(SystemLogHelper::logLevelToString(LogLevel::Info), static_cast<int>(LogLevel::Info));
    ui->logLevel->addItem(SystemLogHelper::logLevelToString(LogLevel::Warning), static_cast<int>(LogLevel::Warning));
    ui->logLevel->addItem(SystemLogHelper::logLevelToString(LogLevel::Error), static_cast<int>(LogLevel::Error));
    ui->logLevel->addItem(SystemLogHelper::logLevelToString(LogLevel::SpecificTips), static_cast<int>(LogLevel::SpecificTips));

    connect(ui->exportBtn, &QPushButton::clicked, this, &SystemLogWidget::slotExportBtnClicked);
    connect(ui->selectBtn, &QPushButton::clicked, this, &SystemLogWidget::slotQueryBtnClicked);
    connect(ui->logLevel, &QComboBox::currentTextChanged, this, &SystemLogWidget::logLevelChange);
    connect(ui->pagesize, &QComboBox::currentTextChanged, this, &SystemLogWidget::pageSizeChange);

    d_pageNavigator = new PageNavigator(3, this);
    ui->horizontalLayout_2->addWidget(d_pageNavigator);
    ui->horizontalLayout_2->addWidget(ui->exportBtn);

    connect(d_pageNavigator, &PageNavigator::currentPageChanged, this, &SystemLogWidget::currentPageChanged);

    //    ui->exportBtn->setHidden(true);

    ui->tableView->setColumnWidth(0, 240);
    ui->tableView->setColumnWidth(1, 60);
    ui->tableView->setColumnWidth(2, 80);
    ui->tableView->setColumnWidth(3, 250);
}

void SystemLogWidget::queryLogs()
{
    SystemLogCondition systemLogCondition;

    systemLogCondition.currentPage = 1;
    systemLogCondition.pageSize = ui->pagesize->currentData().toInt();

    QDateTime startTime = ui->startTime->dateTime();
    QDateTime endTime = ui->endTime->dateTime();
    if (startTime > endTime)
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    systemLogCondition.startTime = startTime.toString(DATETIME_ORIGIN_FORMAT);
    systemLogCondition.endTime = endTime.toString(DATETIME_ORIGIN_FORMAT);
    systemLogCondition.level = LogLevel(ui->logLevel->currentData().toInt());

    QByteArray data;
    data << systemLogCondition;
    emit signalLogConditionQuery(data);
}

void SystemLogWidget::currentPageChanged(int page)
{
    SystemLogCondition systemLogCondition;

    systemLogCondition.currentPage = page;
    systemLogCondition.pageSize = ui->pagesize->currentData().toInt();

    QDateTime startTime = ui->startTime->dateTime();
    QDateTime endTime = ui->endTime->dateTime();
    if (startTime > endTime)
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    systemLogCondition.startTime = startTime.toString(DATETIME_ORIGIN_FORMAT);
    systemLogCondition.endTime = endTime.toString(DATETIME_ORIGIN_FORMAT);
    systemLogCondition.level = LogLevel(ui->logLevel->currentData().toInt());

    QByteArray data;
    data << systemLogCondition;
    emit signalLogConditionQuery(data);
}

void SystemLogWidget::slotExportBtnClicked()
{
    auto systemLogDatas = mSystemLogTableModel->getSystemLogData();
    if (!systemLogDatas.isEmpty())
    {
        QString filename = QFileDialog::getSaveFileName(this, QString("导出系统日志"),
                                                        QString("%1.txt").arg(GlobalData::currentDateTime().toString(DATETIME_ORIGIN_FORMAT)),
                                                        tr("Text files (*.txt)"));
        if (!filename.isEmpty())
        {
            QFile file(filename);
            if (!file.open(QFile::WriteOnly | QFile::Text))
            {
                return;
            }
            QTextStream stream(&file);
            for (auto data : systemLogDatas)
            {
                auto id = data.id;
                auto createTime = data.createTime;
                auto level = SystemLogHelper::logLevelToString(data.level);
                auto type = SystemLogHelper::logTypeString(data.type);
                auto module = data.module;
                auto userID = data.userID;
                auto context = data.context;

                stream /*<< QString("(序号)" + id).toUtf8() << ','*/ << QString("(时间)" + createTime).toUtf8() << ','
                                                                     << QString("(日志等级)" + level).toUtf8() << ','
                                                                     << QString("(日志类型)" + type).toUtf8() << ','
                                                                     << QString("(模块)" + module).toUtf8() << ','
                                                                     << QString("(用户ID)" + userID).toUtf8() << ','
                                                                     << QString("(内容)" + context).toUtf8() << '\n';
            }
            file.close();
        }
    }
}

void SystemLogWidget::slotQueryBtnClicked() { queryLogs(); }
