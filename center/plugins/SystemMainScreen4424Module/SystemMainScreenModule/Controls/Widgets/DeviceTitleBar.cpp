#include "DeviceTitleBar.h"

#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "ServiceHelper.h"
#include <QElapsedTimer>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>

constexpr const char* onlineQss = "QLabel {"
                                  "border:1px solid rgb(85, 170, 255);"
                                  "border-radius:1px;"
                                  "background-color: rgb(85, 170, 255);"
                                  "color: rgb(255, 255, 255);"
                                  "font-size:16px;"
                                  "font-weight:bold;"
                                  "min-height: 30px;"
                                  "}";

constexpr const char* offlineQss = "QLabel {"
                                   "border:1px solid rgb(151, 172, 187);"
                                   "border-radius:1px;"
                                   "background-color: rgb(151, 172, 187);"
                                   "color: rgb(255, 255, 255);"
                                   "font-size:16px;"
                                   "font-weight:bold;"
                                   "min-height: 30px;"
                                   "}";
static QMap<QString, QDialog*> widgetCaches = QMap<QString, QDialog*>();
static QString iconPaths = "";

DeviceTitleBar::DeviceTitleBar(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    initLayout();

    parseNode(domEle);

    setStyleSheet(offlineQss);
}

void DeviceTitleBar::setTitle(const QString& title) { mTitleLab->setText(title); }

QString DeviceTitleBar::getTitle() const { return mTitleLab->text(); }

void DeviceTitleBar::addExtensionMenuAction(const int id, const QString& text)
{
    auto action = new QAction(text, mMenu);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, [=]() { emit signalExtensionActionClicked(id); });

    mMenu->addAction(action);
}

void DeviceTitleBar::setCheckedExtensionAction(const QString& actionText)
{
    for (auto action : mMenu->actions())
    {
        if (action->text() == actionText)
        {
            action->setChecked(true);
            action->setEnabled(false);
        }
        else
        {
            action->setChecked(false);
            action->setEnabled(true);
        }
    }
}

void DeviceTitleBar::setDeviceInfo(const DeviceInfo& deviceInfo) { m_deviceInfo = deviceInfo; }

bool DeviceTitleBar::isOnline() const { return mIsOnline; }

void DeviceTitleBar::setOnline(bool isOnline)
{
    if (mIsOnline == isOnline)
    {
        return;
    }

    mIsOnline = isOnline;
    setStyleSheet(isOnline ? onlineQss : offlineQss);
}

QSize DeviceTitleBar::sizeHint() const { return mTitleLab->sizeHint(); }

QSize DeviceTitleBar::minimumSizeHint() const { return mTitleLab->minimumSizeHint(); }

void DeviceTitleBar::showSystemDialog()
{
    QString systemName = m_deviceInfo.m_extensionSymbolTitle;
    QElapsedTimer timer;
    timer.start();
    auto containerWidget = widgetCaches.value(systemName);
    int deviceID = m_deviceInfo.m_extensionID;
    DeviceID id(deviceID);
    if (containerWidget == nullptr)
    {
        auto module = getService<IGuiService>(m_deviceInfo.m_triggerModuleTitle);
        if (module == nullptr)
        {
            QMessageBox::warning(this, "提示", "加载服务模块失败", "确定");
            return;
        }
        auto widget = module->createWidget(systemName);
        if (widget == nullptr)
        {
            QMessageBox::critical(nullptr, QString("错误"), QString("打开失败"), QString("确定"));
            return;
        }
        widget->setObjectName(systemName);

        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(widget);
        vLayout->setMargin(0);
        containerWidget = new QDialog;
        containerWidget->setMinimumSize(1600, 900);
        containerWidget->setLayout(vLayout);

        QString windowTitle = GlobalData::getSystemName(id.sysID);
        containerWidget->setWindowTitle(windowTitle);
        containerWidget->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        iconPaths = PlatformConfigTools::configBusiness("RibbonIcon/");
        containerWidget->setWindowIcon(QIcon(iconPaths + m_deviceInfo.m_iconName));
        widgetCaches.insert(systemName, containerWidget);
    }
    QWidget* child = containerWidget->findChild<QWidget*>(systemName);
    child->setProperty("deviceID", deviceID);

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
}

void DeviceTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (event->button() == Qt::RightButton)
        {
            mMenu->exec(QCursor::pos());
        }
    }

    BaseWidget::mouseReleaseEvent(event);
}

void DeviceTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        showSystemDialog();
    }

    BaseWidget::mouseDoubleClickEvent(event);
}

void DeviceTitleBar::initLayout()
{
    mMenu = new QMenu(this);

    mTitleLab = new QLabel(this);

    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    mTitleLab->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(mTitleLab);
}

void DeviceTitleBar::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
}
