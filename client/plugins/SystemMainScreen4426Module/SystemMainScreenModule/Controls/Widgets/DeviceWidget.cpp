#include "DeviceWidget.h"

#include <QStackedWidget>
#include <QVBoxLayout>

#include "CustomPacketMessageDefine.h"
#include "DeviceTitleBar.h"
#include "ExtensionWidget.h"
#include "GlobalData.h"
#include "MainScreenControlFactory.h"

DeviceWidget::DeviceWidget(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)

{
    mTitleBar = new DeviceTitleBar(domEle, this);

    initLayout();

    parseNode(domEle);

    // 添加分机widget
    for (auto widget : mExtenWidgets)
    {
        mTitleBar->addExtensionMenuAction(widget->getExtensionId(), widget->getExtensionName());
        mExtenStackedWidget->addWidget(widget);

        connect(widget, &ExtensionWidget::signalOnlineStatusChanged, this, &DeviceWidget::slotExtensionOnlineStatusChanged);
        connect(widget, &ExtensionWidget::signalModeChanged, this, &DeviceWidget::slotExtensionModeChanged);
    }

    setStyleSheet("DeviceWidget {"
                  "border: 2px solid rgb(151, 172, 187);"
                  "border-radius: 5px;"
                  "}");
}

DeviceWidget::~DeviceWidget() {}

void DeviceWidget::initLayout()
{
    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    vLayout->addWidget(mTitleBar);

    mExtenStackedWidget = new QStackedWidget(this);
    vLayout->addWidget(mExtenStackedWidget);

    // 分机菜单点击时
    connect(mTitleBar, &DeviceTitleBar::signalExtensionActionClicked, [=](const int id) {
        for (auto widget : mExtenWidgets)
        {
            if (widget->getExtensionId() == id)
            {
                mExtenStackedWidget->setCurrentWidget(widget);
                return;
            }
        }
    });

    // 当前显示的分机交换时
    connect(mExtenStackedWidget, &QStackedWidget::currentChanged, this, [=]() {
        auto extenWidget = qobject_cast<ExtensionWidget*>(mExtenStackedWidget->currentWidget());

        mTitleBar->setCheckedExtensionAction(extenWidget->getExtensionName());
        mTitleBar->setTitle(extenWidget->getExtensionName() + extenWidget->getCurrentModeName());
        //设置分机数据
        DeviceInfo deviceInfo;
        deviceInfo.m_extensionSymbolTitle = extenWidget->getExtensionSymbol();
        deviceInfo.m_triggerModuleTitle = extenWidget->getExtensionTriggerModule();
        deviceInfo.m_extensionID = extenWidget->getExtensionId();
        deviceInfo.m_iconName = extenWidget->getIcon();
        mTitleBar->setDeviceInfo(deviceInfo);

        // 修改头部样式
        mTitleBar->setOnline(extenWidget->isOnline());
    });
}

void DeviceWidget::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            QString tagName = subDomElement.tagName();

            // 设备分机控件
            if (tagName == "ExtensionWidget")
            {
                auto widget = MainScreenControlFactory::createControl(tagName, subDomElement);
                if (widget != nullptr)
                {
                    auto extenWidget = qobject_cast<ExtensionWidget*>(widget);
                    if (extenWidget != nullptr)
                    {
                        mExtenWidgets << extenWidget;
                    }
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

void DeviceWidget::slotExtensionOnlineStatusChanged(bool isOnline)
{
    // 如果当前显示的分机在线
    auto currentExtenWidget = qobject_cast<ExtensionWidget*>(mExtenStackedWidget->currentWidget());
    if (currentExtenWidget->isOnline())
    {
        mTitleBar->setOnline(isOnline);
        return;
    }

    auto changedExtenWidget = qobject_cast<ExtensionWidget*>(sender());
    // 分机状态为在线，设置显示该分机
    if (isOnline)
    {
        mExtenStackedWidget->setCurrentWidget(changedExtenWidget);
    }
    mTitleBar->setOnline(isOnline);
}

void DeviceWidget::slotExtensionModeChanged()
{
    auto extenWidget = qobject_cast<ExtensionWidget*>(sender());
    if (extenWidget == mExtenStackedWidget->currentWidget())
    {
        mTitleBar->setTitle(extenWidget->getExtensionName() + extenWidget->getCurrentModeName());
        //设置分机数据
        DeviceInfo deviceInfo;
        deviceInfo.m_extensionSymbolTitle = extenWidget->getExtensionSymbol();
        deviceInfo.m_triggerModuleTitle = extenWidget->getExtensionTriggerModule();
        deviceInfo.m_extensionID = extenWidget->getExtensionId();
        deviceInfo.m_iconName = extenWidget->getIcon();
        mTitleBar->setDeviceInfo(deviceInfo);
    }
}
