#include "MainScreenControlFactory.h"

#include "CheckBox.h"
#include "ComboBoxStatusLabel.h"
#include "ComplexCheckBox.h"
#include "ComplexLEDStatusLabel.h"
#include "ComplexNumberStatusLabel.h"
#include "DeviceParamWidgetGenerator.h"
#include "DeviceWidget.h"
#include "DynamicStatusLabel.h"
#include "EnumStatusLabel.h"
#include "ExtensionWidget.h"
#include "GridLayout.h"
#include "GroupBox.h"
#include "LEDStatusDecodeLabel.h"
#include "LEDStatusLabel.h"
#include "LEDStatusStimuLabel.h"
#include "ModeWidget.h"
#include "MultiNumberStatusLabel.h"
#include "MutexCheckBox.h"
#include "NameLabel.h"
#include "NumberStatusFreqLabel.h"
#include "NumberStatusLabel.h"
#include "NumberStatusPowerLabel.h"
#include "NumberStatusRotationLabel.h"
#include "ProtocolXmlTypeDefine.h"
#include "StarMap.h"
#include "VoltageRadar4424.h"
#include "VoltageRadar4426.h"

QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> MainScreenControlFactory::mWidgetMap;
QList<QWidget*> MainScreenControlFactory::mControlWidget;
QMap<QString, QWidget*> MainScreenControlFactory::mMultiStatusLabels;
QList<QWidget*> MainScreenControlFactory::mDynamicParamWidget;
QList<int> MainScreenControlFactory::mDynamicDevids;

QWidget* MainScreenControlFactory::createControl(const QString& type, const QDomElement& domEle, QWidget* parent)
{
    static int currentDeviceId = -1;
    static int currenModeId = -1;

    QWidget* widget = nullptr;
    if (type == "GridLayout")
    {
        widget = new GridLayout(domEle, parent);
    }
    else if (type == "GroupBox")
    {
        widget = new GroupBox(domEle, parent);
    }
    else if (type == "DeviceWidget")
    {
        widget = new DeviceWidget(domEle, parent);
    }
    else if (type == "ExtensionWidget")
    {
        // 保存当前分机的设备Id
        currentDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
        auto extenWidget = new ExtensionWidget(domEle, parent);

        widget = extenWidget;
    }
    else if (type == "ModeWidget")
    {
        // 保存当前模式id
        currenModeId = domEle.attribute("modeId").toInt(nullptr, 16);
        auto modeWidget = new ModeWidget(domEle, parent);

        widget = modeWidget;
    }
    else if (type == "Include")
    {
        auto element = DeviceParamWidgetGenerator::getDomElement(domEle.attribute("path"));
        widget = createControl(element.tagName(), element);
    }
    else if (type == "Widget")
    {
        widget = createWidget(domEle.attribute("type"), domEle, currentDeviceId, currenModeId, parent);
    }

    return widget;
}

QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> MainScreenControlFactory::getWidgetMap() { return mWidgetMap; }

QList<QWidget*> MainScreenControlFactory::getControlWidget() { return mControlWidget; }

QMap<QString, QWidget*> MainScreenControlFactory::getMultiStatusLabels() { return mMultiStatusLabels; }

QList<QWidget*> MainScreenControlFactory::getDynamicParamWidget() { return mDynamicParamWidget; }

QList<int> MainScreenControlFactory::getDynamicDevids() { return mDynamicDevids; }

QWidget* MainScreenControlFactory::createWidget(const QString& type, const QDomElement& domEle, const int& deviceId, const int& modeId,
                                                QWidget* parent)
{
    //需要批量刷新的控件，才需要调用函数appendWidget
    QWidget* widget = nullptr;
    if (type == "NameLabel")
    {
        widget = new NameLabel(domEle, parent);
        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "LEDStatusLabel")
    {
        widget = new LEDStatusLabel(domEle, parent);
        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "ComboBoxStatusLabel")
    {
        widget = new ComboBoxStatusLabel(domEle, parent);
        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "EnumStatusLabel")
    {
        widget = new EnumStatusLabel(domEle, parent, deviceId, modeId);
        EnumStatusLabel* enumStatusLabel = dynamic_cast<EnumStatusLabel*>(widget);
        const int curDeviceId = enumStatusLabel->getDeviceId();
        const int curModeId = enumStatusLabel->getModeId();
        appendWidget(domEle, widget, curDeviceId, curModeId);
    }
    else if (type == "NumberStatusLabel")
    {
        widget = new NumberStatusLabel(domEle, parent);
        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "CheckBox")
    {
        widget = new CheckBox(domEle, parent, deviceId, modeId);
        appendWidget(domEle, widget, deviceId, modeId);
        CheckBox* checkBox = dynamic_cast<CheckBox*>(widget);
        if (!checkBox->isControlType())
            appendControlWidget(widget);
    }
    else if (type == "ComplexCheckBox")
    {
        widget = new ComplexCheckBox(domEle, parent, deviceId, modeId);
        appendWidget(domEle, widget, deviceId, modeId);
        ComplexCheckBox* checkBox = dynamic_cast<ComplexCheckBox*>(widget);
        if (!checkBox->getIsControl())
            appendControlWidget(widget);
    }
    else if (type == "MutexCheckBox")
    {
        widget = new MutexCheckBox(domEle, parent, deviceId, modeId);
        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "StarMap")
    {
        widget = new StarMap(domEle, parent, deviceId, modeId);
        //        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "VoltageRadar4424")
    {
        widget = new VoltageRadar4424(domEle, parent);
        //        appendWidget(domEle, widget, deviceId, modeId);
    }
    else if (type == "VoltageRadar4426")
    {
        widget = new VoltageRadar4426(domEle, parent);
    }
    else if (type == "MultiNumberStatusLabel")
    {
        widget = new MultiNumberStatusLabel(domEle, parent, deviceId, modeId);
        appendMultiStatusLabel(domEle, widget, deviceId, modeId);
    }
    else if (type == "DynamicStatusLabel")
    {
        widget = new DynamicStatusLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        appendDyanamicDevids(0x4001);
        appendDyanamicDevids(0x4002);
        appendDyanamicDevids(0x4003);
    }
    else if (type == "LEDStatusDecodeLabel")
    {
        widget = new LEDStatusDecodeLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }
    else if (type == "NumberStatusPowerLabel")
    {
        widget = new NumberStatusPowerLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }
    else if (type == "LEDStatusStimuLabel")
    {
        widget = new LEDStatusStimuLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }
    else if (type == "NumberStatusFreqLabel")
    {
        widget = new NumberStatusFreqLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }
    else if (type == "NumberStatusRotationLabel")
    {
        widget = new NumberStatusRotationLabel(domEle, parent);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }
    else if (type == "ComplexNumberStatusLabel")
    {
        widget = new ComplexNumberStatusLabel(domEle, parent, deviceId, modeId);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
        //        widget->show();
    }
    else if (type == "ComplexLEDStatusLabel")
    {
        widget = new ComplexLEDStatusLabel(domEle, parent, deviceId, modeId);
        appendDynamicParamWidget(widget);
        QVariantList variantList = widget->property("devids").toList();
        for (QVariant& variant : variantList)
        {
            appendDyanamicDevids(variant.toInt());
        }
    }

    return widget;
}

void MainScreenControlFactory::appendWidget(const QDomElement& domEle, QWidget* widget, const int& curParentDeviceId, const int& curParentModeId)
{
    // 如果该控件没有单元id，表示该控件是不需要值刷新的控件
    if (!domEle.hasAttribute("unitId"))
    {
        return;
    }

    int deviceId = -1;
    int modeId = 0xFFFF;
    // 如果该控件定义了deviceId，那么使用该标签定义的，如果没有自动使用该控件的当前所在的父设备Id
    if (!domEle.hasAttribute("deviceId"))
    {
        deviceId = curParentDeviceId;
    }
    else
    {
        deviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    }

    if (!domEle.hasAttribute("modeId"))
    {
        modeId = curParentModeId;
    }
    else
    {
        modeId = domEle.attribute("modeId", "FFFF").toInt(nullptr, 16);
    }

    auto unitId = domEle.attribute("unitId").toInt();
    auto targetId = domEle.attribute("targetId", "0").toInt();
    auto paramId = domEle.attribute("paramId");

    widget->setObjectName(QString("%1-%2-%3-%4-%5").arg(deviceId).arg(modeId).arg(unitId).arg(targetId).arg(paramId));

    mWidgetMap[deviceId][modeId][unitId][targetId][paramId] = widget;
}

void MainScreenControlFactory::appendMultiStatusLabel(const QDomElement& domEle, QWidget* widget, const int& curParentDeviceId,
                                                      const int& curParentModeId)
{
    // 如果该控件没有单元id，表示该控件是不需要值刷新的控件
    if (!domEle.hasAttribute("unitId"))
    {
        return;
    }

    int deviceId = -1;
    int modeId = 0xFFFF;
    // 如果该控件定义了deviceId，那么使用该标签定义的，如果没有自动使用该控件的当前所在的父设备Id
    if (!domEle.hasAttribute("deviceId"))
    {
        deviceId = curParentDeviceId;
    }
    else
    {
        deviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    }

    if (!domEle.hasAttribute("modeId"))
    {
        modeId = curParentModeId;
    }
    else
    {
        modeId = domEle.attribute("modeId", "FFFF").toInt(nullptr, 16);
    }

    auto unitId = domEle.attribute("unitId").toInt();
    auto targetId = domEle.attribute("targetId", "0").toInt();

    //这里组键的时候，舍弃掉参数。主要因为需要刷新的参数是发令计数，只需要四个参数就可以唯一确定控件了
    QString key = QString("%1_%2_%3_%4")
                      .arg(QString::number(deviceId))
                      .arg(QString::number(modeId))
                      .arg(QString::number(unitId))
                      .arg(QString::number(targetId));
    mMultiStatusLabels[key] = widget;
}

void MainScreenControlFactory::appendControlWidget(QWidget* widget) { mControlWidget.append(widget); }

void MainScreenControlFactory::appendDynamicParamWidget(QWidget* widget) { mDynamicParamWidget.append(widget); }

void MainScreenControlFactory::appendDyanamicDevids(const int devid)
{
    if (!mDynamicDevids.contains(devid))
    {
        mDynamicDevids.append(devid);
    }
}
