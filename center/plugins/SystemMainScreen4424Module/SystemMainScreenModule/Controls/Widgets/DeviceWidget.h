#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QVariant>

#include "BaseWidget.h"
#include "ProtocolXmlTypeDefine.h"

class DeviceTitleBar;
class ExtensionWidget;
class QStackedWidget;
// 设备控件
class DeviceWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit DeviceWidget(const QDomElement& domEle, QWidget* parent = nullptr);
    ~DeviceWidget();

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private slots:
    void slotExtensionOnlineStatusChanged(bool isOnline);
    void slotExtensionModeChanged();

private:
    DeviceTitleBar* mTitleBar = nullptr;

    QStackedWidget* mExtenStackedWidget = nullptr;
    QList<ExtensionWidget*> mExtenWidgets;
};

#endif  // DEVICEWIDGET_H
