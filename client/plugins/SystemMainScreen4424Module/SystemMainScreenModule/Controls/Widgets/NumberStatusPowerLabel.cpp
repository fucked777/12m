#include "NumberStatusPowerLabel.h"
#include <QUuid>

NumberStatusPowerLabel::NumberStatusPowerLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void NumberStatusPowerLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
    setText(valueString);
    setStatus(Normal);
}

QVariant NumberStatusPowerLabel::getValue() const { return QVariant(); }

void NumberStatusPowerLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString NumberStatusPowerLabel::getDealType() const { return d_dealType; }

void NumberStatusPowerLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString NumberStatusPowerLabel::getUniqueKey() const { return d_uniqueKey; }

void NumberStatusPowerLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList NumberStatusPowerLabel::getDevids() { return d_deviceIds; }

QList<NumberStatusPowerLabel::SumInfo> NumberStatusPowerLabel::getParamInfos() { return d_SumInfos; }

void NumberStatusPowerLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = "All_Power_Number_Status";
    d_uniqueKey = QUuid::createUuid().toString();
    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        SumInfo sumInfo;
        auto subChildNode = subDomEle.firstChild();
        while (!subChildNode.isNull())
        {
            auto subChildEle = subChildNode.toElement();
            if (subChildEle.isNull())
            {
                continue;
            }

            if (subChildEle.tagName() == "ParamInfo")
            {
                ParamInfo paramInfo;
                int iDeviceId = subChildEle.attribute("deviceId").toInt(nullptr, 16);
                DeviceID id;
                id << iDeviceId;
                paramInfo.deviceId = id;
                paramInfo.modeId = subChildEle.attribute("modeId").toInt(nullptr, 16);
                paramInfo.unitId = subChildEle.attribute("unitId").toInt(nullptr, 16);
                paramInfo.targetId = subChildEle.attribute("targetId").toInt(nullptr, 16);
                paramInfo.paramId = subChildEle.attribute("paramId");
                sumInfo.paramInfo = paramInfo;
            }
            if (subChildEle.tagName() == "MainInfo")
            {
                MainInfo mainInfo;
                int iDeviceId = subChildEle.attribute("deviceId").toInt(nullptr, 16);
                DeviceID id;
                id << iDeviceId;
                mainInfo.deviceId = id;
                mainInfo.modeId = subChildEle.attribute("modeId").toInt(nullptr, 16);
                mainInfo.unitId = subChildEle.attribute("unitId").toInt(nullptr, 16);
                mainInfo.targetId = subChildEle.attribute("targetId").toInt(nullptr, 16);
                mainInfo.paramId = subChildEle.attribute("paramId");
                mainInfo.value = subChildEle.attribute("value");
                sumInfo.mainInfo = mainInfo;
            }
            subChildNode = subChildNode.nextSibling();
        }
        d_SumInfos.append(sumInfo);
        domNode = domNode.nextSibling();
    }

    int cd = 0;
}
