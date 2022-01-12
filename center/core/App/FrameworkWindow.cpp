#include "FrameworkWindow.h"
#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"
#include "PlatfromDefine.h"
#include "ServiceInfoMap.h"
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QDockWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>

class FrameworkWindowImpl
{
public:
    BundleContext context;
    ListenerToken listenerToken;
    QString ribbonIconPath;
    QList<MenuInfo> menuInfos;      // 菜单信息
    QList<ModuleInfo> moduleInfos;  // 模块信息

    /* 服务的映射信息 */
    ServiceInfoMap serviceInfoMap;
    //    /* 这两个需要同步更新 */
    //    QMap<QString, QWidget*> mWidgetMap; /* 菜单名与控件的映射窗口 */
    //    /* 模块与控件的映射,这里List是考虑一个模块可以创建多个界面的 */
    //    QMap<void*, QList<QWidget*>> mServiceToWidgetMap;

    QList<QAction*> defaultTriggeredActions;  // 启动默认触发的菜单

  explicit  FrameworkWindowImpl(cppmicroservices::BundleContext context_)
        : context(context_)
    {
    }
};

FrameworkWindow::FrameworkWindow(cppmicroservices::BundleContext context, QWidget* parent)
    : QMainWindow(parent)
    , m_impl(new FrameworkWindowImpl(context))
{
    qRegisterMetaType<ServiceReferenceU>("ServiceReferenceU");
    m_impl->ribbonIconPath = PlatformConfigTools::configBusiness("RibbonIcon/");

    m_impl->listenerToken = m_impl->context.AddServiceListener([=](const ServiceEvent& event) {
        std::string objectClass = ref_any_cast<std::vector<std::string>>(event.GetServiceReference().GetProperty(Constants::OBJECTCLASS)).front();

        if (event.GetType() == ServiceEvent::SERVICE_REGISTERED)
        {
            qDebug() << "Ex1: Service of type " << QString::fromStdString(objectClass) << " registered.";
            signalInstallGuiServices(event.GetServiceReference());
        }
        else if (event.GetType() == ServiceEvent::SERVICE_UNREGISTERING)
        {
            qDebug() << "Ex1: Service of type " << QString::fromStdString(objectClass) << " unregistered.";

            signalUninstallGuiServices(event.GetServiceReference());
        }
        else if (event.GetType() == ServiceEvent::SERVICE_MODIFIED)
        {
            qDebug() << "Ex1: Service of type " << QString::fromStdString(objectClass) << " modified.";
            signalUninstallGuiServices(event.GetServiceReference());
            signalInstallGuiServices(event.GetServiceReference());
        }
    });
    connect(this, &FrameworkWindow::signalUninstallGuiServices, this, &FrameworkWindow::uninstallGuiServices, Qt::QueuedConnection);

    /*
     * qApp->setStyleSheet("QTabBar::tab {"
                        "height: 40px;"
                        "color: #303133;"
                        "background-color: white;"
                        "border: 1px solid #e4e7ed;"
                        "border-bottom: none;"
                        "padding-left: 20px;"
                        "padding-right: 20px;"
                        "}"
                        "QTabBar::tab:hover {"
                        "color: #409eff;"
                        "}"
                        "QTabBar::tab:selected {"
                        "color: #409eff;"
                        "border-bottom: 2px solid #409eff;"
                        "}"

                        "QGroupBox {"
                        "border: 2px solid #c7c7c7;"
                        "border-radius: 5px;"
                        "margin-top: 1ex;"
                        "}"
                        "QGroupBox::title {"
                        "subcontrol-origin: margin;"
                        "subcontrol-position: top left; "
                        "padding: 0;"
                        "left: 6px;"
                        "}"

                        "QPushButton {"
                        "color:white;"
                        "border-radius: 6px;"
                        "min-width: 80px;"
                        "min-height: 30px;"
                        "}"
                        "QPushButton:enabled {"
                        "background-color: #409eff;"
                        "}"
                        "QPushButton:disabled {"
                        "background-color: #909399;"
                        "}"
                        "QPushButton:hover {"
                        "background-color: #66b1ff;"
                        "}"
                        "QPushButton:pressed {"
                        "background-color: #409eff;"
                        "}"

                        "QToolTip {"
                        "background-color: white;"
                        "border: 2px solid white;"
                        "color: black;"
                        "}"

                        "QScrollBar:vertical {"
                        "width: 6px;"
                        "background-color: transparent;"
                        "margin: 16px 0 16px 0;"
                        "}"
                        "QScrollBar::handle:vertical {"
                        "background: #dddee0;"
                        "min-height: 16px;"
                        "border-radius: 2px;"
                        "}"
                        "QScrollBar::handle:vertical:hover {"
                        "background: #c7c9cc;"
                        "}"
                        "QScrollBar::add-line:vertical {"
                        "background-color: red;"
                        "height: 0px;"
                        "}"
                        "QScrollBar::sub-line:vertical {"
                        "background-color: red;"
                        "height: 0px;"
                        "}"

    );
    */
}

FrameworkWindow::~FrameworkWindow() {}

void FrameworkWindow::setModuleInfo(const QList<ModuleInfo>& infoList) { m_impl->moduleInfos = infoList; }

void FrameworkWindow::setMenuInfo(const QList<MenuInfo>& menuInfos) { m_impl->menuInfos = menuInfos; }

void FrameworkWindow::init()
{
    resize(qApp->desktop()->maximumSize());

    install();

    for (const auto& menuInfo : m_impl->menuInfos)
    {
        if (menuInfo.subMenus.isEmpty())
        {
            auto action = new QAction(menuInfo.name, nullptr);
            connect(action, &QAction::triggered, this, [=]() { slotMenuActionClicked(menuInfo); });
            menuBar()->addAction(action);

            if (menuInfo.isDefaultTriggered)
            {
                m_impl->defaultTriggeredActions.append(action);
            }

            continue;
        }

        auto menu = new QMenu(menuInfo.name);
        createMenu(menuInfo.subMenus, menu);

        menuBar()->addMenu(menu);
    }
}

void FrameworkWindow::showEvent(QShowEvent* event)
{
    static bool isFirstShow = true;
    if (isFirstShow)
    {
        isFirstShow = false;
        for (auto action : m_impl->defaultTriggeredActions)
        {
            emit action->triggered(true);
        }
    }

    QMainWindow::showEvent(event);
}

void FrameworkWindow::install()
{
    for (auto& moduleInfo : m_impl->moduleInfos)
    {
        if (!moduleInfo.use)
        {
            continue;
        }
        auto& moduleName = moduleInfo.name;
        auto moduleFullPath = PlatformConfigTools::plugPath(moduleName);
        /* 插件不存在 */
        if (!QFile::exists(moduleFullPath))
        {
            qWarning() << QString("插件不存在：%1").arg(moduleName);
            continue;
        }

        try
        {
            auto newBundles = m_impl->context.InstallBundles(moduleFullPath.toStdString());
            for (auto& newBundle : newBundles)
            {
                newBundle.Start();
            }
        }
        catch (const std::exception& ex)
        {
            qWarning() << QString("安装插件失败：%1，%2").arg(moduleName).arg(ex.what());
            continue;
        }
        catch (...)
        {
            qWarning() << QString("安装插件失败");
            continue;
        }
    }
}

void FrameworkWindow::createMenu(const QList<MenuInfo>& menuInfos, QMenu* parentMenu)
{
    for (auto& menuInfo : menuInfos)
    {
        if (menuInfo.subMenus.isEmpty())
        {
            auto action = new QAction(menuInfo.name, nullptr);
            connect(action, &QAction::triggered, this, [=]() { slotMenuActionClicked(menuInfo); });
            parentMenu->addAction(action);
        }
        else
        {
            auto menu = new QMenu(menuInfo.name);
            createMenu(menuInfo.subMenus, menu);
            parentMenu->addMenu(menu);
        }
    }
}

void FrameworkWindow::uninstallGuiServices(ServiceReferenceU serviceRef)
{
    auto guiService = m_impl->context.GetService<IGuiService>(serviceRef);

    if (guiService == nullptr)
    {
        return;
    }
    m_impl->serviceInfoMap.unstallService(guiService.get());
}

void FrameworkWindow::slotMenuActionClicked(const MenuInfo& menuInfo)
{
    auto serviceRefs = m_impl->context.GetServiceReferences("");

    /* 1. 查找服务*/
    std::shared_ptr<IGuiService> curGuiService;
    for (auto serviceRef : serviceRefs)
    {
        /* 模块名一致 且 是gui服务 */
        if ((QString("[IGuiService]") == QString::fromStdString(serviceRef.GetProperty("objectclass").ToString())) &&
            (QString::fromStdString(serviceRef.GetBundle().GetSymbolicName()) == menuInfo.moduleName))
        {
            curGuiService = m_impl->context.GetService<IGuiService>(serviceRef);
            break;
        }
    }
    if (curGuiService == nullptr)
    {
        return;
    }
    /* 2 根据服务找和菜单名字找控widget  */
    /* 这里这么写是希望,失败就不更改原有数据 */
    auto containerWidget = m_impl->serviceInfoMap.getWidget(curGuiService.get(), menuInfo.name);

    /* 3 根据类型区分 */
    switch (menuInfo.widgetType)
    {
    case MenuWidgetType::Dock:  // dock窗口
    {
        /* 首次使用 */
        /* 这里没有原来的逻辑,因为qobject_cast也存在失败的可能就无法分辨是本来就是nullptr还是转换失败了 */
        if (containerWidget == nullptr)
        {
            auto tempWidget = curGuiService->createWidget(menuInfo.subWidgetName);
            if (tempWidget == nullptr)
            {
                QMessageBox::critical(this, QString("错误"), QString("加载当前界面失败"), "确认");
                return;
            }

            auto dockWidget = new QDockWidget(menuInfo.name, this);
            m_impl->serviceInfoMap.insertWidget(curGuiService.get(), menuInfo.name, dockWidget);
            dockWidget->setWidget(tempWidget);
            dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
            dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
            addDockWidget(Qt::RightDockWidgetArea, dockWidget);
            containerWidget = dockWidget;
        }
        containerWidget->setVisible(!containerWidget->isVisible());
        return;
    }
    case MenuWidgetType::Dialog:  // dialog窗口
    {
        /* 这里没有原来的逻辑,因为qobject_cast也存在失败的可能就无法分辨是本来就是nullptr还是转换失败了 */
        if (containerWidget == nullptr)
        {
            auto tempWidget = curGuiService->createWidget(menuInfo.subWidgetName);
            if (tempWidget == nullptr)
            {
                QMessageBox::critical(this, QString("错误"), QString("加载当前界面失败"), "确认");
                return;
            }

            auto vLayout = new QVBoxLayout;
            vLayout->addWidget(tempWidget);

            vLayout->setMargin(0);

            containerWidget = new QDialog;
            containerWidget->setMinimumSize(menuInfo.size);

            containerWidget->setLayout(vLayout);
            containerWidget->setWindowTitle(menuInfo.name);
            containerWidget->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint |
                                            Qt::WindowMaximizeButtonHint);  // 只显示关闭按钮

            containerWidget->setWindowIcon(QIcon(m_impl->ribbonIconPath + menuInfo.iconName));

            m_impl->serviceInfoMap.insertWidget(curGuiService.get(), menuInfo.name, containerWidget);
        }

        containerWidget->show();
        return;
    }
    case MenuWidgetType::UnknownType:
    {
        break;
    }
    }

    /* 到这里都是加载失败 */
    QMessageBox::critical(nullptr, QString("错误"), QString("菜单窗口类型配置错误"), "确认");
}
