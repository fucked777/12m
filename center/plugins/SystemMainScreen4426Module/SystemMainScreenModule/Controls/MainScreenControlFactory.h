#ifndef MAINSCREENCONTROLFACTORY_H
#define MAINSCREENCONTROLFACTORY_H

#include <QDomElement>
#include <QList>
#include <QMap>
#include <QWidget>

class MainScreenControlFactory
{
public:
    // 创建控件
    static QWidget* createControl(const QString& type, const QDomElement& domEle, QWidget* parent = nullptr);

    static QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> getWidgetMap();

    //这里的控件指的是测控基带标准和扩二的checkbox,需要发命令。在控件内部已经把命令打包好了，然后通过信号传出去。这里提供获取控件的接口就是为了绑定信号和槽
    static QList<QWidget*> getControlWidget();

    //这里的控件指的是发令计数，需要取4个参数的值，然后连接起来做刷新
    static QMap<QString, QWidget*> getMultiStatusLabels();

    static QList<QWidget*> getDynamicParamWidget();

    static QList<int> getDynamicDevids();

private:
    static QWidget* createWidget(const QString& type, const QDomElement& domEle, const int& deviceId, const int& modeId, QWidget* parent = nullptr);

    //主要可以直接获取得到参数值
    static void appendWidget(const QDomElement& domEle, QWidget* widget, const int& curParentDeviceId, const int& curParentModeId);

    //同时需要刷新4个参数的控件,可以放在特殊处理里面
    static void appendMultiStatusLabel(const QDomElement& domEle, QWidget* widget, const int& curParentDeviceId, const int& curParentModeId);

    //需要发送命令的控件
    static void appendControlWidget(QWidget* widget);

    //特殊处理
    static void appendDynamicParamWidget(QWidget* widget);

    static void appendDyanamicDevids(const int devid);

private:
    // QMap<设备id, QMap<模式id, QMap<单元id, QMap<目标id, QMap<QString, QWidget*>>>>>
    static QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> mWidgetMap;
    static QList<QWidget*> mControlWidget;
    //需要特殊处理的刷新控件
    static QMap<QString, QWidget*> mMultiStatusLabels;
    static QList<QWidget*> mDynamicParamWidget;
    static QList<int> mDynamicDevids;
};

#endif  // MAINSCREENCONTROLFACTORY_H
