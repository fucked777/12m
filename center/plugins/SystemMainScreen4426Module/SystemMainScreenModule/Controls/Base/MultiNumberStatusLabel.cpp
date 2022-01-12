#include "MultiNumberStatusLabel.h"

#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"

MultiNumberStatusLabel::MultiNumberStatusLabel(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : StatusLabel(domEle, parent)
    , d_curValue("")
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    parseNode(domEle);
    //    startTimer(1000);
}

void MultiNumberStatusLabel::setUnitValue(const QVariant& value)
{
    d_values.clear();
    QMap<QString, QVariant> unitValue = value.toMap();
    for (auto& paramInfo : d_paramsInfo)
    {
        d_values.append(unitValue.value(paramInfo.paramId).toString());
    }

    if (d_curValue != d_values.join("/"))
    {
        d_curValue = d_values.join("/");
        setText(d_curValue);
    }
}

QVariant MultiNumberStatusLabel::getUnitValue() const { return QVariant(); }

// void MultiNumberStatusLabel::timerEvent(QTimerEvent* event)
//{
//    Q_UNUSED(event)
//    BaseWidget::timerEvent(event);

//    d_values.clear();

//    for (auto& paramInfo : d_paramsInfo)
//    {
//        auto statusReportMsg = GlobalData::getExtenStatusReportData(paramInfo.deviceId);

//        if (statusReportMsg.modeId == -1)
//            continue;

//        QVariant paramValue;
//        if (paramInfo.targetId == 0)
//            paramValue = statusReportMsg.unitReportMsgMap[paramInfo.unitId].paramMap[paramInfo.paramId];
//        else
//            paramValue = statusReportMsg.unitReportMsgMap[paramInfo.unitId].multiTargetParamMap[paramInfo.targetId][paramInfo.paramId];

//        d_values.append(paramValue.toString());
//    }

//    if (d_curValue != d_values.join("/"))
//    {
//        d_curValue = d_values.join("/");
//        setText(d_curValue);
//    }
//}

void MultiNumberStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    if (domEle.hasAttribute("width"))
    {
        int width = domEle.attribute("width").toInt();
        setMinWidth(width);
    }
    auto domNode = domEle.firstChild();

    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        auto tagName = subDomEle.tagName();
        if (tagName == "Status")
        {
            ParamsInfo info;
            info.deviceId = d_deviceId;
            info.modeId = d_modeId;

            info.unitId = subDomEle.attribute("unitId").toInt(nullptr, 16);
            info.paramId = subDomEle.attribute("paramId");

            if (subDomEle.hasAttribute("targetId"))
                info.targetId = subDomEle.attribute("targetId").toInt(nullptr, 16);
            else
                info.targetId = 0;
            d_paramsInfo.append(info);
        }
        domNode = domNode.nextSibling();
    }
}
