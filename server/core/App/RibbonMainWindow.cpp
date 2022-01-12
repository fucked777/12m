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

    //    testPack();
}

RibbonMainWindow::~RibbonMainWindow() {}
void RibbonMainWindow::closeApplication()
{
    mContext.RemoveListener(std::move(mListenerToken));
    mListenerToken = ListenerToken();
}
#if 0
#include "ControlCmdResponseUnpacker.h"
#include "ControlResultReportUnpacker.h"
#include "ExtensionStatusReportUnpacker.h"
#include "PacketHandler.h"
#include "ProcessControlCmdPacker.h"
#include "UnitParamSetPacker.h"

#include <QVBoxLayout>
void RibbonMainWindow::testPack()
{
    auto widget = new QWidget;
    setCentralWidget(widget);

    PacketHandler::instance().appendPacker(new UnitParamSetPacker());
    PacketHandler::instance().appendPacker(new ProcessControlCmdPacker());
    PacketHandler::instance().appendUnpacker(new ControlResultReportUnpacker());
    PacketHandler::instance().appendUnpacker(new ExtensionStatusReportUnpacker());
    PacketHandler::instance().appendUnpacker(new ControlCmdResponseUnpacker());

    auto vLayout = new QVBoxLayout;
    widget->setLayout(vLayout);

    auto btn = new QPushButton("分机状态上报");
    vLayout->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, [=]() {
        auto data = QByteArray::fromHex(
            "0000000160244226000000010024422604f100000000000017062120bace3914a90100000024fb0102020102020100000000000001011100003030544553540201443a5c"
            "544b2d34343234c1aacad4d3a6b4f0bbfa5cb1ead7bc5454435c446174615ccad4cad4cac700000000000000000000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fe01fb0201fb04a701bcf9430300000000801d"
            "2c04030564004600fe02fb03801d2c0402e20202014646012c0101fe03fb040201160000010001000000000400000201020101010201a9000000ff000000088000080003"
            "1dfccf1a01010800010101000290eb0000fe04fb050100010100801d2c04c0fcad7b32000100010100801d2c0480461c861e00fe05");

        UnpackMessage msg;
        PacketHandler::instance().unpack(data, msg);
    });

    auto ctrlResultBtn = new QPushButton("控制结果上报");
    vLayout->addWidget(ctrlResultBtn);
    connect(ctrlResultBtn, &QPushButton::clicked, this, [=]() {
        auto data = QByteArray::fromHex("0000000140244226000000010024422601f10000120200000000212060557d132e000000fb4223010000303030303031310006072120"
                                        "2a813812780000002f0000003700000012000000170000001b000000");

        UnpackMessage msg;
        PacketHandler::instance().unpack(data, msg);
    });

    auto btn1 = new QPushButton("控制命令响应");
    vLayout->addWidget(btn1);
    connect(btn1, &QPushButton::clicked, this, [=]() {
        auto data = QByteArray::fromHex("0000000420264226000000010026422600f1000013020000290721209bcca9040b00000001f0000005480a00000000");

        UnpackMessage msg;
        PacketHandler::instance().unpack(data, msg);
    });

    auto btn2 = new QPushButton("过程控制命令");
    vLayout->addWidget(btn2);
    connect(btn2, &QPushButton::clicked, this, [=]() {
        ProcessControlCmdMessage controlCtrlCmdMsg;
        controlCtrlCmdMsg.mode = 0x1;
        controlCtrlCmdMsg.cmdId = 4;

        QMap<QString, QVariant> map;

        map["EpochDate"] = "2021/07/05";
        map["EpochTime"] = "20:25:29:508";
        map["ETrackEccentricity"] = 12;
        map["ITrackAngle"] = 2;
        map["PerigeeAngle"] = 3;
        map["RightAscendNode"] = 4;
        map["HorizontalAngle"] = 5;

        controlCtrlCmdMsg.settingParamMap = map;

        MessageAddress targetAddr;
        targetAddr.systemNumb = 0x4;
        targetAddr.deviceNumb = 0x0;
        targetAddr.extenNumb = 0x1;

        PackMessage packMsg;
        packMsg.header.msgType = DevMsgType::ProcessControlCmd;
        packMsg.header.sourceAddr = targetAddr;
        packMsg.header.targetAddr = targetAddr;
        packMsg.processCtrlCmdMsg = controlCtrlCmdMsg;

        QByteArray data;
        QString errorMsg;
        PacketHandler::instance().pack(packMsg, data, errorMsg);
    });
}
#endif
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
                        else if (widget && containerWidget)
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
