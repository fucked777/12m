#include "SCUtility.h"
#include "CustomPacketMessageDefine.h"
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QRegularExpression>

void SetWidgetValue::setLabelText(const QVariant& value, WidgetValueInfo& info)
{
    auto widget = qobject_cast<QLabel*>(info.widget);
    if (widget != nullptr)
    {
        widget->setText(value.toString());
    }
}
void SetWidgetValue::setLineEditText(const QVariant& value, WidgetValueInfo& info)
{
    auto widget = qobject_cast<QLineEdit*>(info.widget);
    if (widget != nullptr)
    {
        widget->setText(value.toString());
    }
}

void SetWidgetValue::setDoubleLabelText(const QVariant& value, WidgetValueInfo& info)
{
    auto widget = qobject_cast<QLabel*>(info.widget);
    if (widget == nullptr)
    {
        return;
    }
    auto rawValue = value.toString();

    QRegularExpression regex(R"(^-?[0-9]\d*\.?\d{0,3}$)");
    auto match = regex.match(rawValue);
    if (match.hasMatch())
    {
        widget->setText(match.captured(0));
    }
    else
    {
        widget->setText(rawValue);
    }
}
void SetWidgetValue::setLampGRG(const QVariant& value, WidgetValueInfo& info)
{
    /*
     * svalue="0" uvalue="0" desc="无此参数"  灰色
     * svalue="1" uvalue="1" desc="故障"      红色
     * svalue="2" uvalue="2" desc="正常"      绿色
     */
    auto widget = qobject_cast<QLabel*>(info.widget);
    if (widget == nullptr)
    {
        return;
    }

    auto propertyNum = widget->property("LedAttr").toInt();
    auto valueNum = value.toInt();
    if (valueNum == propertyNum)
    {
        return;
    }

    auto pixmap = (valueNum == 2) ? QPixmap(":/images/green.png") : ((valueNum == 1) ? QPixmap(":/images/red.png") : QPixmap(":/images/gray.png"));
    widget->setPixmap(pixmap);
    widget->setProperty("LedAttr", valueNum);
}
void SetWidgetValue::setLampGGR(const QVariant& value, WidgetValueInfo& info)
{
    /*
     * svalue="0" uvalue="0" desc="无效"  灰色
     * svalue="1" uvalue="1" desc="锁定"  绿色
     * svalue="2" uvalue="2" desc="失锁"  红色
     */
    auto widget = qobject_cast<QLabel*>(info.widget);
    if (widget == nullptr)
    {
        return;
    }

    auto propertyNum = widget->property("LedAttr").toInt();
    auto valueNum = value.toInt();
    if (valueNum == propertyNum)
    {
        return;
    }

    auto pixmap = (valueNum == 1) ? QPixmap(":/images/green.png") : ((valueNum == 2) ? QPixmap(":/images/red.png") : QPixmap(":/images/gray.png"));
    widget->setPixmap(pixmap);
    widget->setProperty("LedAttr", valueNum);
}

void SetWidgetValue::setLampGRYG(const QVariant& value, WidgetValueInfo& info)
{
    /*
     * 0：无此参数    灰色
     * 1：未启动       红色
     * 2：开始        绿色
     * 3：完成        绿色
     */
    auto widget = qobject_cast<QLabel*>(info.widget);
    if (widget == nullptr)
    {
        return;
    }

    auto propertyNum = widget->property("LedAttr").toInt();
    auto valueNum = value.toInt();
    if (valueNum == propertyNum)
    {
        return;
    }

    auto pixmap = (valueNum == 2 || valueNum == 3) ? QPixmap(":/images/green.png")
                                                   : ((valueNum == 1) ? QPixmap(":/images/red.png") : QPixmap(":/images/gray.png"));
    widget->setPixmap(pixmap);
    widget->setProperty("LedAttr", valueNum);
}

void SCUtility::resetLedStatus(QLabel* led)
{
    auto gray = QPixmap(":/images/gray.png");
    led->setPixmap(gray);
    led->setProperty("LedAttr", 0);
}

void SCUtility::registerValueInfo(const QString& key, QWidget* widget, int unit, SetStatusFunc func, QList<WidgetValueInfo>& info)
{
    info << createValueInfo(key, widget, unit, func);
}

WidgetValueInfo SCUtility::createValueInfo(const QString& key, QWidget* widget, int unit, SetStatusFunc func)
{
    WidgetValueInfo temp;
    temp.key = key;
    temp.widget = widget;
    temp.unit = unit;
    temp.setStatusFunc = func;
    return temp;
}

void SCUtility::bgBlackTextGreenLabel(QWidget* widget)
{
    static constexpr const char* qss = ".QLabel {"
                                       "border:1px solid rgb(0, 0, 0);"
                                       //"border-radius:5px;"
                                       "color: rgb(0, 255, 0);"
                                       "max-height: 28px;"
                                       "min-height: 28px;"
                                       "background-color: rgb(0, 0, 0);"
                                       "font-weight:bold;"
                                       "font-size:16px;"
                                       "}";
    widget->setStyleSheet(qss);
}

void SCUtility::setDataS(const ExtensionStatusReportMessage& message, QList<WidgetValueInfo>& info)
{
    auto& extenReportMap = message.unitReportMsgMap;
    for (auto& item : info)
    {
        auto find = extenReportMap.find(item.unit);
        if (find == extenReportMap.end())
        {
            continue;
        }
        auto& reportMap = find->paramMap;
        auto valueFind = reportMap.find(item.key);
        if (valueFind == reportMap.end())
        {
            continue;
        }
        item.setStatusFunc(valueFind.value(), item);
    }
}
