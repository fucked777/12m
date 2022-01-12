#include "ExtensionWidget.h"

#include <QStackedWidget>
#include <QVBoxLayout>

#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include "MainScreenControlFactory.h"
#include "ModeWidget.h"
#include "ProtocolXmlTypeDefine.h"

ExtensionWidget::ExtensionWidget(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    initLayout();

    parseNode(domEle);

    for (auto modeWidget : mModeWidgets)
    {
        mModeStackedWidget->addWidget(modeWidget);
    }

    startTimer(1000);
}

ExtensionWidget::~ExtensionWidget() {}

int ExtensionWidget::getExtensionId() const { return mExtenId; }

QString ExtensionWidget::getExtensionName() const { return mName; }

QString ExtensionWidget::getExtensionSymbol() const { return m_extensionSymbol; }

QString ExtensionWidget::getExtensionTriggerModule() const { return m_triggerModule; }

QString ExtensionWidget::getIcon() const { return m_icon; }

QString ExtensionWidget::getCurrentModeName() const
{
    auto modeWidget = qobject_cast<ModeWidget*>(mModeStackedWidget->currentWidget());
    if (modeWidget != nullptr)
    {
        return modeWidget->getModeName();
    }

    return QString();
}

void ExtensionWidget::setOnline(bool isOnline)
{
    if (mIsOnline == isOnline)
    {
        return;
    }

    mIsOnline = isOnline;
    emit signalOnlineStatusChanged(isOnline);
}

bool ExtensionWidget::isOnline() const { return mIsOnline; }

void ExtensionWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)
    // 是否在线

    if (mDeviceIds.size() == 0)
    {
        auto isOnline = GlobalData::getDeviceOnlineByObject(mExtenId);
        setOnline(isOnline);

        // 在线就获取分机上报的数据
        if (isOnline)
        {
            auto extenStatusReportMsg = GlobalData::getExtenStatusReportDataByObject(mExtenId);
            if (extenStatusReportMsg.modeId != mCurrentModeId)
            {
                mCurrentModeId = extenStatusReportMsg.modeId;
                // 根据该分机上报模式id切换为对应的ModeWidget
                auto widget = mModeWidgets[extenStatusReportMsg.modeId];
                if (widget != nullptr)
                {
                    mModeStackedWidget->setCurrentWidget(widget);
                    emit signalModeChanged();
                }
            }
        }
    }
    else
    {
        for (int deviceId : mDeviceIds)
        {
            auto isOnline = GlobalData::getDeviceOnlineByObject(deviceId);

            if (isOnline == true)
            {
                setOnline(isOnline);
                return;
            }
        }
        setOnline(false);
    }
}

void ExtensionWidget::initLayout()
{
    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    mModeStackedWidget = new QStackedWidget(this);
    vLayout->addWidget(mModeStackedWidget);
}

void ExtensionWidget::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    mExtenId = domEle.attribute("deviceId").toInt(nullptr, 16);
    mName = domEle.attribute("name");
    m_triggerModule = domEle.attribute("triggerModule").trimmed();
    m_extensionSymbol = domEle.attribute("extensionSymbol").trimmed();
    m_icon = domEle.attribute("icon").trimmed();
    if (domEle.hasAttribute("multiDeviceId"))
    {
        QString deviceIds = domEle.attribute("multiDeviceId");
        for (QString deviceId : deviceIds.split(";"))
        {
            int devid = deviceId.toInt(nullptr, 16);
            mDeviceIds.append(devid);
        }
    }

    QDomNode domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        QDomElement subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            QString tagName = subDomElement.tagName();

            QWidget* widget = MainScreenControlFactory::createControl(tagName, subDomElement);
            if (widget != nullptr)
            {
                auto modeWidget = qobject_cast<ModeWidget*>(widget);
                // 有模式的分机
                if (modeWidget != nullptr)
                {
                    mModeWidgets[modeWidget->getModeId()] = modeWidget;
                }
                // 无模式的分机
                else
                {
                    mModeWidgets[0xFFFF] = widget;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}
