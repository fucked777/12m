#ifndef DEVICEPARAMWIDGETGENERATOR_H
#define DEVICEPARAMWIDGETGENERATOR_H

#include <QDomElement>
#include <QWidget>

// 系统参数监视模块生成器
class DeviceParamWidgetGenerator
{
public:
    static QWidget* getStatusWidget(QWidget* parent = nullptr);
    static QList<QWidget*> getControlWidget();

    // 获取指定xml文件的root节点
    static QDomElement getDomElement(const QString& path);

private:
    void loadPath(const QString& configPath);

private:
    static QString mMainScreenConfigFileDir;
    static QList<QWidget*> mControlWidget;
};

#endif  // DEVICEPARAMWIDGETGENERATOR_H
