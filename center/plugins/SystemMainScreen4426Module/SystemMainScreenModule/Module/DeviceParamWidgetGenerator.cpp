#include "DeviceParamWidgetGenerator.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QMessageBox>

#include "MainScreenControlFactory.h"
#include "PlatformInterface.h"

QString DeviceParamWidgetGenerator::mMainScreenConfigFileDir;
QList<QWidget*> DeviceParamWidgetGenerator::mControlWidget;

QWidget* DeviceParamWidgetGenerator::getStatusWidget(QWidget* parent)
{
    auto fileInfo = QFileInfo(PlatformConfigTools::configBusiness("MainScreen/MainScreen.xml"));
    mMainScreenConfigFileDir = fileInfo.absolutePath();

    auto mainScreenElement = getDomElement(fileInfo.absoluteFilePath());
    if (mainScreenElement.isNull())
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("主界面配置文件为内容为空:%1").arg(fileInfo.absoluteFilePath()), QString("确定"));
        return nullptr;
    }
    auto tagName = mainScreenElement.tagName();

    QWidget* widget = MainScreenControlFactory::createControl(tagName, mainScreenElement, parent);
    mControlWidget = MainScreenControlFactory::getControlWidget();
    return widget;
}

QList<QWidget*> DeviceParamWidgetGenerator::getControlWidget() { return mControlWidget; }

QDomElement DeviceParamWidgetGenerator::getDomElement(const QString& path)
{
    QString configFilePath;

    QFileInfo fileInfo(path);
    if (fileInfo.isAbsolute())
    {
        configFilePath = fileInfo.absoluteFilePath();
    }
    else
    {
        configFilePath = mMainScreenConfigFileDir + QDir::separator() + path;
    }

    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("主界面配置文件打开失败:%1,%2").arg(configFilePath).arg(file.errorString()),
                              QString("确定"));
        return QDomElement();
    }

    QString errorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg, &errorRow, &errorCol))
    {
        file.close();
        QMessageBox::critical(nullptr, QString("警告"),
                              QString("解析XML文件错误:%1,第%3第%4列，%2").arg(configFilePath).arg(errorRow).arg(errorCol).arg(errorMsg),
                              QString("确定"));
        return QDomElement();
    }
    file.close();

    return dom.documentElement();
}
