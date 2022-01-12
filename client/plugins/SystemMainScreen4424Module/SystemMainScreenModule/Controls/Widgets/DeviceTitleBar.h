#ifndef DEVICETITLEBAR_H
#define DEVICETITLEBAR_H

#include <QLabel>

#include "BaseWidget.h"
#include "ProtocolXmlTypeDefine.h"
#include <QMouseEvent>

struct DeviceInfo
{
    int m_extensionID = -1;          //该分机的设备ID
    QString m_triggerModuleTitle;    //触发的模块名
    QString m_iconName;              //图片名字
    QString m_extensionSymbolTitle;  //该分机对应的标识如测控基带,跟踪基带等基带对应JD这一个标识 方面创建界面
};

class QMenu;
struct Device;
// 设备标题栏
class DeviceTitleBar : public BaseWidget
{
    Q_OBJECT
public:
    explicit DeviceTitleBar(const QDomElement& domEle, QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString getTitle() const;

    void addExtensionMenuAction(const int id, const QString& text);
    // 设置选中action
    void setCheckedExtensionAction(const QString& actionText);

    void setDeviceInfo(const DeviceInfo& deviceInfo);

    bool isOnline() const;
    void setOnline(bool isOnline);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void showSystemDialog();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

signals:
    // 菜单点击
    void signalExtensionActionClicked(const int& extendId);

private:
    QLabel* mTitleLab = nullptr;

    bool mIsOnline = false;

    QMenu* mMenu = nullptr;

    DeviceInfo m_deviceInfo;
};

#endif  // DEVICETITLEBAR_H
