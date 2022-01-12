#include "SystemMainScreen.h"
#include "ui_SystemMainScreen.h"

#include "CppMicroServicesUtility.h"
#include "SystemLogWidget.h"
#include "TaskPlanWidget.h"
#include "TimeInfomationWidget.h"

#include <QDockWidget>
#include <QLabel>

SystemMainScreen::SystemMainScreen(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::SystemMainScreen)
{
    ui->setupUi(this);

    initTimeWidget();
    initTaskPlanWidget();
    initSystemLogWidget();

    setStyleSheet("QLabel[flag='titleBar']  {"
                  "qproperty-alignment:AlignCenter;"
                  "border:1px solid rgb(151, 172, 187);"
                  "border-radius:1px;"
                  "background-color: rgb(151, 172, 187);"
                  "color: rgb(255, 255, 255);"
                  "font-size:16px;"
                  "font-weight:bold;"
                  "min-height: 30px;"
                  "}"

                  "QPalette {"
                  "background:#EAF7FF;"
                  "}"
                  "*{"
                  "outline:0px;"
                  "color:#386487;"
                  "}");
}

SystemMainScreen::~SystemMainScreen() { delete ui; }

void SystemMainScreen::initSystemLogWidget()
{
    auto sysLogTitleBar = new QLabel(this);
    sysLogTitleBar->setProperty("flag", "titleBar");
    sysLogTitleBar->setObjectName("sysLogTitleBar");
    sysLogTitleBar->setText("系统日志");

    auto systemLogWidget = new SystemLogWidget(this);
    auto sysLogDockWidget = new QDockWidget(this);
    sysLogDockWidget->setObjectName("sysLogDockWidget");
    sysLogDockWidget->setWindowTitle("系统日志");
    sysLogDockWidget->setTitleBarWidget(sysLogTitleBar);
    sysLogDockWidget->setWidget(systemLogWidget);
    addDockWidget(Qt::BottomDockWidgetArea, sysLogDockWidget);
}

void SystemMainScreen::initTaskPlanWidget()
{
    auto taskPlanTitleBar = new QLabel(this);
    taskPlanTitleBar->setProperty("flag", "titleBar");
    taskPlanTitleBar->setObjectName("titleBar");
    taskPlanTitleBar->setText("任务计划");

    auto taskPlanWidget = new TaskPlanWidget(this);
    auto taskPlanDockWidget = new QDockWidget(this);
    taskPlanDockWidget->setObjectName("taskPlanDockWidget");
    taskPlanDockWidget->setWindowTitle("任务计划");
    taskPlanDockWidget->setTitleBarWidget(taskPlanTitleBar);
    taskPlanDockWidget->setWidget(taskPlanWidget);

    addDockWidget(Qt::TopDockWidgetArea, taskPlanDockWidget);
}

void SystemMainScreen::initTimeWidget()
{
    auto timeInfoTitleBar = new QLabel(this);
    timeInfoTitleBar->setProperty("flag", "titleBar");
    timeInfoTitleBar->setObjectName("timeInfomationWidget");
    timeInfoTitleBar->setText("时间");

    auto timeInfoWidget = new TimeInfomationWidget(this);
    auto timeInfoDockWidget = new QDockWidget(this);
    timeInfoDockWidget->setObjectName("timeInfomationDockWidget");
    timeInfoDockWidget->setWindowTitle("时间");
    timeInfoDockWidget->setTitleBarWidget(timeInfoTitleBar);
    timeInfoDockWidget->setWidget(timeInfoWidget);
    addDockWidget(Qt::TopDockWidgetArea, timeInfoDockWidget);

    connect(timeInfoWidget, &TimeInfomationWidget::signalTimeConfigInfoChanged, this, &SystemMainScreen::signalTimeConfigChanged);
}
