#include "NumberStatusRotationLabel.h"
#include <QUuid>

NumberStatusRotationLabel::NumberStatusRotationLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void NumberStatusRotationLabel::setValue(const QVariant& value)
{
    setText(value.toString());
    setStatus(Normal);
}

QVariant NumberStatusRotationLabel::getValue() const { return QVariant(); }

void NumberStatusRotationLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString NumberStatusRotationLabel::getDealType() const { return d_dealType; }

void NumberStatusRotationLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString NumberStatusRotationLabel::getUniqueKey() const { return d_uniqueKey; }

void NumberStatusRotationLabel::setConnector(const QString& connector) { d_connector = connector; }

QString NumberStatusRotationLabel::getConnector() const { return d_connector; }

void NumberStatusRotationLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList NumberStatusRotationLabel::getDevids() { return d_deviceIds; }

QList<NumberStatusRotationLabel::ParamInfo> NumberStatusRotationLabel::getParamInfos() { return d_SumInfos; }

void NumberStatusRotationLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = "All_Rotation_Number_Status";
    d_uniqueKey = QUuid::createUuid().toString();
    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }
    d_connector = domEle.attribute("connector");

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        if (subDomEle.tagName() == "ParamInfo")
        {
            ParamInfo paramInfo;
            int iDeviceId = subDomEle.attribute("deviceId").toInt(nullptr, 16);
            DeviceID id;
            id << iDeviceId;
            paramInfo.deviceId = id;
            paramInfo.modeId = subDomEle.attribute("modeId").toInt(nullptr, 16);
            paramInfo.unitId = subDomEle.attribute("unitId").toInt(nullptr, 16);
            paramInfo.targetId = subDomEle.attribute("targetId").toInt(nullptr, 16);
            paramInfo.paramId = subDomEle.attribute("paramId");

            QMap<QString, QString> mapping;
            parseEnumNode(subDomEle, mapping);
            paramInfo.mapping = mapping;
            d_SumInfos.append(paramInfo);
        }
        domNode = domNode.nextSibling();
    }
}

void NumberStatusRotationLabel::parseEnumNode(const QDomElement& domEle, QMap<QString, QString>& mapping)
{
    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        QString sValue = subDomEle.attribute("value");
        QString uValue = subDomEle.attribute("desc");
        mapping.insert(sValue, uValue);
        domNode = domNode.nextSibling();
    }
}
