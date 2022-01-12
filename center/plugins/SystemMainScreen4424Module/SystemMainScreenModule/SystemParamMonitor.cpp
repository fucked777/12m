#include "SystemParamMonitor.h"

#include "AutoTaskWidget.h"
#include "DeviceParamWidget.h"
#include "DockWidget.h"
#include "NavBarWidget.h"
#include "PlatformInterface.h"
#include "SystemLogWidget.h"
#include "TaskPlanWidget.h"
#include "TimeWidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QLabel>
#include <QSettings>

class SystemParamMonitorImpl
{
public:
    QString windowStateFile;  // 窗口状态文件路径

    QDockWidget* sysLogDockWidget = nullptr;
    QDockWidget* autoTaskDockWidget = nullptr;
    QDockWidget* taskPlanDockWidget = nullptr;
    QDockWidget* timeDockWidget = nullptr;
};

SystemParamMonitor::SystemParamMonitor(QWidget* parent)
    : QMainWindow(parent)
    , mImpl(new SystemParamMonitorImpl())
{
    mImpl->windowStateFile = PlatformConfigTools::configBusiness("MainScreen/MainScreenState.ini");

    initTimeWidget();
    initTaskPlanWidget();
    initDeviceParamWidget();
    initSystemLogWidget();
    initAutoTaskWidget();

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
}

SystemParamMonitor::~SystemParamMonitor()
{
    //    saveWindowState();

    if (mImpl != nullptr)
    {
        delete mImpl;
        mImpl = nullptr;
    }
}

void SystemParamMonitor::initDeviceParamWidget()
{
    auto paramWidget = new DeviceParamWidget(this);
    bool res = connect(paramWidget, &DeviceParamWidget::signalsCmdDeviceJson, this, &SystemParamMonitor::signalsCmdDeviceJson);
    connect(paramWidget, &DeviceParamWidget::signalsUnitDeviceJson, this, &SystemParamMonitor::signalsUnitDeviceJson);
    setCentralWidget(paramWidget);
}

void SystemParamMonitor::initSystemLogWidget()
{
    auto sysLogTitleBar = new QLabel(this);
    sysLogTitleBar->setProperty("flag", "titleBar");
    sysLogTitleBar->setObjectName("sysLogTitleBar");
    sysLogTitleBar->setText("系统日志");
    auto systemLogWidget = new SystemLogWidget(this);

    mImpl->sysLogDockWidget = new QDockWidget(this);
    mImpl->sysLogDockWidget->setObjectName("sysLogDockWidget");
    mImpl->sysLogDockWidget->setWindowTitle("系统日志");
    mImpl->sysLogDockWidget->setTitleBarWidget(sysLogTitleBar);
    mImpl->sysLogDockWidget->setWidget(systemLogWidget);
    addDockWidget(Qt::BottomDockWidgetArea, mImpl->sysLogDockWidget);

    // 当系统日志置为顶层时，第一次默认初始化一个大小
    connect(mImpl->sysLogDockWidget, &QDockWidget::topLevelChanged, this, [=](bool topLevel) {
        static bool isFirstTopLevel = true;
        if (topLevel && isFirstTopLevel)
        {
            isFirstTopLevel = false;
            int w = 800, h = 600;
            auto desktopWidget = QApplication::desktop();
            auto desktopW = desktopWidget->screen()->width();
            auto desktopH = desktopWidget->screen()->height();
            mImpl->sysLogDockWidget->setGeometry((desktopW - w) / 2, (desktopH - h) / 2, w, h);
        }
    });
}

void SystemParamMonitor::initAutoTaskWidget()
{
    auto autoTaskTitleBar = new QLabel(this);
    autoTaskTitleBar->setProperty("flag", "titleBar");
    autoTaskTitleBar->setObjectName("autoTaskTitleBar");
    autoTaskTitleBar->setText("自动运行");
    auto autoTaskWidget = new AutoTaskWidget(this);

    mImpl->autoTaskDockWidget = new QDockWidget(this);
    mImpl->autoTaskDockWidget->setObjectName("autoTaskDockWidget");
    mImpl->autoTaskDockWidget->setWindowTitle("自动运行");
    mImpl->autoTaskDockWidget->setTitleBarWidget(autoTaskTitleBar);
    mImpl->autoTaskDockWidget->setWidget(autoTaskWidget);
    addDockWidget(Qt::BottomDockWidgetArea, mImpl->autoTaskDockWidget);
}

void SystemParamMonitor::initTaskPlanWidget()
{
    auto taskPlanTitleBar = new QLabel(this);
    //    taskPlanTitleBar->setProperty("flag", "titleBar");
    taskPlanTitleBar->setObjectName("taskPlanTitleBar");
    taskPlanTitleBar->setText("任务计划");
    taskPlanTitleBar->setStyleSheet("QLabel {"
                                    "border:1px solid rgb(151, 172, 187);"
                                    "border-radius:1px;"
                                    "background-color: rgb(151, 172, 187);"
                                    "color: rgb(255, 255, 255);"
                                    "font-size:16px;"
                                    "font-weight:bold;"
                                    "min-height: 30px;"
                                    "}");
    taskPlanTitleBar->setAlignment(Qt::AlignCenter);
    auto taskPlanWidget = new TaskPlanWidget(this);
    mImpl->taskPlanDockWidget = new DockWidget(this);
    mImpl->taskPlanDockWidget->setObjectName("taskPlanDockWidget");
    mImpl->taskPlanDockWidget->setWindowTitle("任务计划");
    mImpl->taskPlanDockWidget->setTitleBarWidget(taskPlanTitleBar);
    mImpl->taskPlanDockWidget->setWidget(taskPlanWidget);
    mImpl->taskPlanDockWidget->setStyleSheet("DockWidget {"
                                             "border: 2px solid rgb(151, 172, 187);"
                                             "border-radius: 1px;"
                                             "}");
    addDockWidget(Qt::TopDockWidgetArea, mImpl->taskPlanDockWidget);
}

void SystemParamMonitor::initTimeWidget()
{
    auto timeTitleBar = new QLabel(this);
    //    timeTitleBar->setProperty("flag", "titleBar");
    timeTitleBar->setObjectName("timeTitleBar");
    timeTitleBar->setText("时间");
    timeTitleBar->setStyleSheet("QLabel {"
                                "border:1px solid rgb(151, 172, 187);"
                                "border-radius:1px;"
                                "background-color: rgb(151, 172, 187);"
                                "color: rgb(255, 255, 255);"
                                "font-size:16px;"
                                "font-weight:bold;"
                                "min-height: 30px;"
                                "}");
    timeTitleBar->setAlignment(Qt::AlignCenter);
    auto timeWidget = new TimeWidget(this);

    mImpl->timeDockWidget = new DockWidget(this);
    mImpl->timeDockWidget->setObjectName("timeDockWidget");
    mImpl->timeDockWidget->setWindowTitle("时间");
    mImpl->timeDockWidget->setTitleBarWidget(timeTitleBar);
    mImpl->timeDockWidget->setWidget(timeWidget);
    mImpl->timeDockWidget->setStyleSheet("DockWidget {"
                                         "border: 2px solid rgb(151, 172, 187);"
                                         "border-radius: 1px;"
                                         "}");
    addDockWidget(Qt::TopDockWidgetArea, mImpl->timeDockWidget);
}

void SystemParamMonitor::initWindowState()
{
    QSettings settings(mImpl->windowStateFile, QSettings::IniFormat);
    settings.beginGroup("SystemParamMonitor");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
    settings.endGroup();
}

void SystemParamMonitor::saveWindowState()
{
    QSettings settings(mImpl->windowStateFile, QSettings::IniFormat);
    settings.beginGroup("SystemParamMonitor");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    settings.endGroup();
}

void SystemParamMonitor::showEvent(QShowEvent* event)
{
    static bool isFirstShow = true;
    if (isFirstShow)
    {
        isFirstShow = false;
        //        initWindowState();

        // 这里延迟一会进行设置dock大小，直接设置会失效？？？
        QTimer::singleShot(100, this, [=]() {
            QList<QDockWidget*> dockList = { mImpl->sysLogDockWidget, mImpl->autoTaskDockWidget };
            QList<int> sizes = { 10, 1 };
            QMainWindow::resizeDocks(dockList, sizes, Qt::Horizontal);
        });
    }
}
