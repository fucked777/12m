#include "RibbonMainWindow.h"

#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"
#include "SARibbonBar/SARibbonBar.h"
#include "SARibbonBar/SARibbonButtonGroupWidget.h"
#include "SARibbonBar/SARibbonCategory.h"
#include "SARibbonBar/SARibbonCheckBox.h"
#include "SARibbonBar/SARibbonComboBox.h"
#include "SARibbonBar/SARibbonGallery.h"
#include "SARibbonBar/SARibbonLineEdit.h"
#include "SARibbonBar/SARibbonMenu.h"
#include "SARibbonBar/SARibbonPannel.h"
#include "SARibbonBar/SARibbonQuickAccessBar.h"
#include "SARibbonBar/SARibbonToolButton.h"
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

extern uint g_uiRemoteClose;
void WriteLog(QString strLog, unsigned char ucWarningLevel = 1);
RibbonMainWindow::RibbonMainWindow(cppmicroservices::BundleContext context, QWidget* parent)
    : SARibbonMainWindow(parent)
    , mContext(context)

{
    m_ribbonIconPath = PlatformConfigTools::configBusiness("RibbonIcon/");

    SARibbonBar* ribbon = ribbonBar();

    ribbonBar()->setRibbonStyle(SARibbonBar::WpsLiteStyle);
    QFont f = ribbon->font();
    f.setFamily("微软雅黑");
    ribbon->setFont(f);

    ribbon->applitionButton()->hide();
    m_pMdiArea = new QMdiArea(this);
    setCentralWidget(m_pMdiArea);

    qRegisterMetaType<ServiceReferenceU>("ServiceReferenceU");
    mListenerToken = mContext.AddServiceListener([=](const ServiceEvent& event) {
        std::string objectClass = ref_any_cast<std::vector<std::string>>(event.GetServiceReference().GetProperty(Constants::OBJECTCLASS)).front();

        auto curType = event.GetType();
        if (curType == ServiceEvent::SERVICE_REGISTERED)
        {
            // emit signalInstallGuiServices(event.GetServiceReference());
        }
        else if (curType == ServiceEvent::SERVICE_UNREGISTERING)
        {
            emit signalUninstallGuiServices(event.GetServiceReference());
        }
        else if (curType == ServiceEvent::SERVICE_MODIFIED)
        {
            emit signalUninstallGuiServices(event.GetServiceReference());
            // emit signalInstallGuiServices(event.GetServiceReference());
        }
    });
    connect(this, &RibbonMainWindow::signalUninstallGuiServices, this, &RibbonMainWindow::uninstallGuiServices, Qt::QueuedConnection);
    // connect(this, &RibbonMainWindow::signalInstallGuiServices, this, &RibbonMainWindow::installGuiServices, Qt::QueuedConnection);
    setWindowFlags(Qt::FramelessWindowHint);
}

RibbonMainWindow::~RibbonMainWindow() {}
void RibbonMainWindow::closeApplication()
{
    mContext.RemoveListener(std::move(mListenerToken));
    mListenerToken = ListenerToken();
}
void RibbonMainWindow::closeEvent(QCloseEvent* event)
{
    if (0 != QMessageBox::information(this, "提示", "是否要退出本程序！！！！", "退出", "取消"))
    {
        event->ignore();
        return;
    }
    auto serviceRefs = mContext.GetServiceReferences("");
    for (auto& serviceRef : serviceRefs)
    {
        if (QString("[IGuiService]") == serviceRef.GetProperty("objectclass").ToString().c_str())
        {
            for (const auto& menuInfo : mMenuInfos)
            {
                if (menuInfo.subMenus.isEmpty())
                {
                    closeMenu(menuInfo);
                }
                else
                {
                    for (const auto& submenuInfo : menuInfo.subMenus)
                    {
                        closeMenu(submenuInfo);
                    }
                }
            }
        }
    }

    event->accept();
}

void RibbonMainWindow::closeMenu(MenuInfo menuInfo)
{
    auto serviceRefs = mContext.GetServiceReferences("");
    for (auto serviceRef : serviceRefs)
    {
        if (QString("[IGuiService]") == serviceRef.GetProperty("objectclass").ToString().c_str())
        {
            auto guiService = mContext.GetService<IGuiService>(serviceRef).get();
            if (guiService != nullptr && QString(serviceRef.GetBundle().GetSymbolicName().c_str()) == menuInfo.moduleName)
            {
                auto containerWidget = serviceInfoMap.getWidget(guiService, menuInfo.name);
                if (nullptr != containerWidget)
                {
                    containerWidget->close();
                }
            }
        }
    }
}

void RibbonMainWindow::setModuleInfo(const QList<ModuleInfo>& infoList) { mModuleInfos = infoList; }
void RibbonMainWindow::setMenuInfo(const QList<MenuInfo>& menuInfos) { mMenuInfos = menuInfos; }
void RibbonMainWindow::init()
{
    for (const auto& menuInfo : mMenuInfos)
    {
        if (menuInfo.subMenus.isEmpty())
        {
            auto action = new QAction(menuInfo.name, this);
            connect(action, &QAction::triggered, this, [=]() { slotMenuActionClicked(menuInfo); });
            menuBar()->addAction(action);
            continue;
        }
        SARibbonBar* ribbon = ribbonBar();
        SARibbonCategory* categoryMain = ribbon->addCategoryPage(menuInfo.name);
        SARibbonPannel* panne2 = categoryMain->addPannel(menuInfo.name);
        createMenu(menuInfo.subMenus, panne2);
    }
}

void RibbonMainWindow::createMenu(const QList<MenuInfo>& menuInfos, SARibbonPannel* pannel)
{
    for (auto& menuInfo : menuInfos)
    {
        if (menuInfo.subMenus.isEmpty())
        {
            auto action = new QAction(menuInfo.name, this);
            if (!menuInfo.pngSrc.isEmpty())
            {
                auto icon = QIcon(QPixmap(menuInfo.pngSrc).scaled(75, 75));
                action->setIcon(icon);
            }
            connect(action, &QAction::triggered, this, [=]() { slotMenuActionClicked(menuInfo); });
            auto btn = pannel->addLargeAction(action);
            btn->setFixedHeight(78);
            // pannel->addLargeAction(action);

            if (QString("用户登录") == menuInfo.name)
            {
                m_pLoginMenuInfo = (MenuInfo*)&menuInfo;
            }

            if (menuInfo.isDefaultTriggered)
            {
                defaultMenuInfo = menuInfo;
            }
        }
        else
        {
            auto action = new QAction(menuInfo.name, this);
            if (!menuInfo.pngSrc.isEmpty())
            {
                auto icon = QIcon(QPixmap(menuInfo.pngSrc).scaled(75, 75));
                action->setIcon(icon);
            }
            connect(action, &QAction::triggered, this, [=]() { slotMenuActionClicked(menuInfo); });
            auto btn = pannel->addLargeAction(action);
            btn->setFixedHeight(78);
            // pannel->addLargeAction(action);
        }
    }
}

// void RibbonMainWindow::installGuiServices(ServiceReferenceU serviceRef)
//{
//    if (QString("[IGuiService]") == serviceRef.GetProperty("objectclass").ToString().c_str())
//    {
//        auto guiService = mContext.GetService<IGuiService>(serviceRef).get();
//        if (guiService != nullptr)
//        {
//            // guiService->createWidget(this);
//        }
//    }
//}

void RibbonMainWindow::uninstallGuiServices(ServiceReferenceU serviceRef)
{
    auto guiService = mContext.GetService<IGuiService>(serviceRef);

    if (guiService != nullptr)
    {
        serviceInfoMap.unstallService(guiService.get());
    }
}

void RibbonMainWindow::slotMenuActionClicked(const MenuInfo& menuInfo)
{
    auto serviceRefs = mContext.GetServiceReferences("");
    for (auto serviceRef : serviceRefs)
    {
        if (QString("[IGuiService]") == serviceRef.GetProperty("objectclass").ToString().c_str())
        {
            auto guiService = mContext.GetService<IGuiService>(serviceRef).get();
            if (guiService != nullptr && QString(serviceRef.GetBundle().GetSymbolicName().c_str()) == menuInfo.moduleName)
            {
                auto containerWidget = serviceInfoMap.getWidget(guiService, menuInfo.name);

                switch (menuInfo.widgetType)
                {
                case MenuWidgetType::Dock:  // dock窗口
                {
                    if (nullptr == containerWidget)
                    {
                        QWidget* widget = guiService->createWidget(menuInfo.subWidgetName);
                        if (widget == nullptr)
                        {
                            QMessageBox::critical(nullptr, QString("错误"), QString("打开失败"), QString("确定"));
                            return;
                        }
                        QMdiSubWindow* subWindow = new QMdiSubWindow(m_pMdiArea);
                        subWindow->setWindowFlags((Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::FramelessWindowHint));
                        subWindow->setAccessibleName(menuInfo.moduleName);
                        subWindow->setWidget(widget);
                        subWindow->showMaximized();
                        m_pMdiArea->setActiveSubWindow(subWindow);
                        containerWidget = subWindow;
                        serviceInfoMap.insertWidget(guiService, menuInfo.name, containerWidget);
                    }
                    else
                    {
                        auto widget = qobject_cast<QMdiSubWindow*>(containerWidget);
                        widget->showMaximized();
                        m_pMdiArea->setActiveSubWindow(widget);
                    }
                    return;
                }
                case MenuWidgetType::Dialog:  // dialog窗口
                {
                    if (containerWidget == nullptr)
                    {
                        QWidget* widget = guiService->createWidget(menuInfo.subWidgetName);
                        if (widget == nullptr)
                        {
                            // QMessageBox::critical(nullptr, QString("错误"), QString("打开失败"), QMessageBox::Ok);
                            return;
                        }

                        auto vLayout = new QVBoxLayout;
                        vLayout->addWidget(widget);
                        vLayout->setMargin(0);
                        containerWidget = new QDialog;
                        containerWidget->setMinimumSize(menuInfo.size);
                        containerWidget->setLayout(vLayout);
                        containerWidget->setWindowTitle(menuInfo.name);
                        containerWidget->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
                        containerWidget->setWindowIcon(QIcon(m_ribbonIconPath + menuInfo.iconName));

                        serviceInfoMap.insertWidget(guiService, menuInfo.name, containerWidget);

                        if (QString("UserLogin") == menuInfo.subWidgetName)
                        {
                            m_pLoginDialog = (QDialog*)containerWidget;
                            return;
                        }
                    }
                    //这里是做一个处理，用户注册和用户列表只有在当前管理员登录的情况下才能打开界面，不然就直接返回nullptr给一个提示框
                    if (menuInfo.subWidgetName == "UserRegister" || menuInfo.subWidgetName == "UserList")
                    {
                        auto widget = guiService->createWidget(menuInfo.subWidgetName);
                        if (widget == nullptr)
                        {
                            return;
                        }
                        else if (containerWidget)
                        {
                            delete widget;
                            widget = nullptr;
                        }
                    }
                    if (containerWidget)
                    {
                        if (containerWidget->isHidden())
                        {
                            containerWidget->show();
                        }
                        if (containerWidget->isMinimized())
                        {
                            containerWidget->showNormal();
                        }
                        // 置于顶层，并设置为活动窗口获取焦点
                        containerWidget->raise();
                        containerWidget->activateWindow();
                    }

                    return;
                }
                default:
                {
                    QMessageBox::critical(nullptr, QString("错误"), QString("菜单窗口类型配置错误"), QString("确定"));
                    return;
                }
                }
            }
        }
    }
}

void RibbonMainWindow::showEvent(QShowEvent* event) { slotMenuActionClicked(defaultMenuInfo); }
#include <QThread>
void RibbonMainWindow::login()
{
    while (!isVisible())
    {
        QThread::msleep(100);
        //_sleep(100);
    }
    if (nullptr != m_pLoginMenuInfo)
    {
        slotMenuActionClicked(*m_pLoginMenuInfo);
        m_pLoginMenuInfo = nullptr;

        if (nullptr != m_pLoginDialog)
        {
            m_pLoginDialog->show();
            m_pLoginDialog = nullptr;
        }
    }
}

void RibbonMainWindow::onShowContextCategory(bool on) { ribbonBar()->setContextCategoryVisible(m_contextCategory, on); }

void RibbonMainWindow::onMenuButtonPopupCheckableTest(bool b) { Q_UNUSED(b) }

void RibbonMainWindow::onInstantPopupCheckableTest(bool b) { Q_UNUSED(b) }

void RibbonMainWindow::onDelayedPopupCheckableTest(bool b) { Q_UNUSED(b) }

void RibbonMainWindow::onMenuButtonPopupCheckabletriggered(bool b) { Q_UNUSED(b) }

void RibbonMainWindow::onInstantPopupCheckabletriggered(bool b) { Q_UNUSED(b) }

void RibbonMainWindow::onDelayedPopupCheckabletriggered(bool b) { Q_UNUSED(b) }
