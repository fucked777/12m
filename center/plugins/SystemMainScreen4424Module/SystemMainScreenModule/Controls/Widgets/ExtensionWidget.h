#ifndef EXTENSIONWIDGET_H
#define EXTENSIONWIDGET_H

#include "BaseWidget.h"

#include <QMap>

class ModeWidget;

class QStackedWidget;
// 分机控件
class ExtensionWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit ExtensionWidget(const QDomElement& domEle, QWidget* parent = nullptr);
    ~ExtensionWidget();

    int getExtensionId() const;
    QString getExtensionName() const;
    QString getExtensionSymbol() const;
    QString getExtensionTriggerModule() const;
    QString getIcon() const;

    // 获取当前显示的分机模式名称
    QString getCurrentModeName() const;

    // 当前分机是否在线
    void setOnline(bool isOnline);
    bool isOnline() const;

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

signals:
    // 分机状态改变
    void signalOnlineStatusChanged(bool isOnline);
    // 分机模式改变
    void signalModeChanged();

private:
    QStackedWidget* mModeStackedWidget = nullptr;

    int mExtenId = 0;       // 分机设备id
    QList<int> mDeviceIds;  //多个分机设备
    QString mName;          // 分机名称
    bool mIsOnline = false;
    int mCurrentModeId = 0;  // 分机的当前模式

    QString m_icon;             //图片名字
    QString m_triggerModule;    //触发的模块名
    QString m_extensionSymbol;  //该分机对应的标识如测控基带,跟踪基带等基带对应JD这一个标识 方面创建界面

    QMap<int, QWidget*> mModeWidgets;
};

#endif  // EXTENSIONWIDGET_H
