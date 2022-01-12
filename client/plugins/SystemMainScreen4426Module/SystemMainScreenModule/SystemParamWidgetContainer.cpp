#include "SystemParamWidgetContainer.h"

#include "ExtendedConfig.h"
#include "NavBarWidget.h"
#include "PlatformInterface.h"
#include "ServiceHelper.h"
#include "SystemParamMonitor.h"

#include <QDockWidget>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QStackedWidget>

class SystemParamWidgetContainerImpl
{
public:
    NavBarWidget* navBarWidget = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QList<std::tuple<QString, QString, QWidget*>> widgets;  // 需要进行生成的界面 QList<std::tuple<导航栏名称, 模块名, 点击时的界面>>
    QMap<int, QWidget*> generatedWidgetMap;                 // 已经生成过的界面  QMap<导航栏对应的行, 是否生成>
    QString indowStateFile;                                 // 窗口状态文件路径

    SystemParamMonitor* systemParamMonitor;
};

SystemParamWidgetContainer::SystemParamWidgetContainer(QWidget* parent)
    : QMainWindow(parent)
    , mImpl(new SystemParamWidgetContainerImpl())
{
    mImpl->indowStateFile = PlatformConfigTools::configBusiness("MainScreen/MainScreenState.ini");

    //    widgets.append(std::make_tuple(QString("设备重组"), "1", new QLabel("设备重组")));
    auto widget = new SystemParamMonitor(this);
    mImpl->systemParamMonitor = widget;

    bool res = connect(widget, &SystemParamMonitor::signalsCmdDeviceJson, this, &SystemParamWidgetContainer::signalsCmdDeviceJson);
    connect(widget, &SystemParamMonitor::signalsUnitDeviceJson, this, &SystemParamWidgetContainer::signalsUnitDeviceJson);
    mImpl->widgets.append(std::make_tuple(QString("系统监控"), "SystemParamMonitor", widget));
    mImpl->widgets.append(std::make_tuple(QString("任务计划"), "TaskPlanManagerModule", nullptr));

    if (ExtendedConfig::curProjectID() == "4424")
    {
        mImpl->widgets.append(std::make_tuple(QString("系统框图"), "SystemBlockDiagramUI4424Module", nullptr));
    }
    if (ExtendedConfig::curProjectID() == "4426")
    {
        mImpl->widgets.append(std::make_tuple(QString("系统框图"), "SystemBlockDiagramUI4426Module", nullptr));
    }

    initNavBarWidget();
    initStackedWidget();

    connect(mImpl->navBarWidget, &NavBarWidget::signalCurrentItemClicked, this, &SystemParamWidgetContainer::slotNavBarItemClicked);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    setStyleSheet("QLabel[flag='titleBar'] {"
                  "min-height: 20px;"
                  "color: rgb(255, 255, 255);"
                  "font-size:16px;"
                  "font-weight:bold;"
                  "background-color: #97acbb;"
                  ""
                  "}");
}

SystemParamWidgetContainer::~SystemParamWidgetContainer()
{
    // saveWindowState();
}

void SystemParamWidgetContainer::initWindowState()
{
    QSettings settings(mImpl->indowStateFile, QSettings::IniFormat);
    settings.beginGroup("SystemParamWidgetContainer");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
    settings.endGroup();
}

void SystemParamWidgetContainer::saveWindowState()
{
    QSettings settings(mImpl->indowStateFile, QSettings::IniFormat);
    settings.beginGroup("SystemParamWidgetContainer");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    settings.endGroup();
}

void SystemParamWidgetContainer::showEvent(QShowEvent* event)
{
    static bool isFirstShow = true;
    if (isFirstShow)
    {
        isFirstShow = false;

        // 选中第1个item(系统监控)
        mImpl->navBarWidget->setCurrentRow(0);

        //        initWindowState();
    }
}

void SystemParamWidgetContainer::initNavBarWidget()
{
    auto navTitleBar = new QLabel(this);
    navTitleBar->setProperty("flag", "titleBar");
    navTitleBar->setObjectName("navTitleBar");
    navTitleBar->setText("导航栏");
    mImpl->navBarWidget = new NavBarWidget(this);
    for (auto tuple : mImpl->widgets)
    {
        mImpl->navBarWidget->appendItem(std::get<0>(tuple), std::get<1>(tuple));
    }

    auto navBarDockWidget = new QDockWidget(this);
    navBarDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    navBarDockWidget->setObjectName("navBarDockWidget");
    navBarDockWidget->setWindowTitle("导航栏");
    navBarDockWidget->setTitleBarWidget(navTitleBar);
    navBarDockWidget->setWidget(mImpl->navBarWidget);
    addDockWidget(Qt::LeftDockWidgetArea, navBarDockWidget);

    resizeDocks({ navBarDockWidget }, { 1 }, Qt::Horizontal);
}

void SystemParamWidgetContainer::initStackedWidget()
{
    mImpl->stackedWidget = new QStackedWidget();
    setCentralWidget(mImpl->stackedWidget);
}

void SystemParamWidgetContainer::slotNavBarItemClicked(QListWidgetItem* item)
{
    auto row = item->listWidget()->row(item);
    /* 先查找缓存的窗体 */
    auto find = mImpl->generatedWidgetMap.find(row);
    if (find != mImpl->generatedWidgetMap.end())
    {
        mImpl->stackedWidget->setCurrentWidget(find.value());
        return;
    }

    auto moduleName = item->data(Qt::UserRole).toString();
    QWidget* widget = nullptr;
    for (auto& tuple : mImpl->widgets)
    {
        if (moduleName == std::get<1>(tuple))
        {
            widget = std::get<2>(tuple);
            break;
        }
    }

    // 为空表示是服务窗体，通过IGuiService进行创建
    if (widget == nullptr)
    {
        /* 20210301处理一下,当模块未加载此处会为空异常结束 */
        auto module = getService<IGuiService>(moduleName);
        if (module == nullptr)
        {
            QMessageBox::warning(this, "提示", "当前模块加载失败", "确定");
            return;
        }
        /* 20210401创建窗体的参数只是用在模块监控上的,对于侧边栏是用不上的,所以写的是空字符串 */
        widget = module->createWidget("");
    }

    if (widget == nullptr)
    {
        return;
    }

    mImpl->generatedWidgetMap[row] = widget;
    mImpl->stackedWidget->insertWidget(row, widget);
    mImpl->stackedWidget->setCurrentWidget(widget);
}
