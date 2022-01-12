#include "ComplexNumberStatusLabel.h"
#include <QUuid>

ComplexNumberStatusLabel::ComplexNumberStatusLabel(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : StatusLabel(domEle, parent)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    parseNode(domEle);
}

void ComplexNumberStatusLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
    setText(valueString);
    setStatus(Normal);
}

QVariant ComplexNumberStatusLabel::getValue() const { return QVariant(); }

void ComplexNumberStatusLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString ComplexNumberStatusLabel::getDealType() const { return d_dealType; }

void ComplexNumberStatusLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString ComplexNumberStatusLabel::getUniqueKey() const { return d_uniqueKey; }

void ComplexNumberStatusLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList ComplexNumberStatusLabel::getDevids() { return d_deviceIds; }

ComplexNumberStatusLabel::SumInfo ComplexNumberStatusLabel::getParamInfos() { return d_SumInfo; }

void ComplexNumberStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = "All_Complex_Number_Status";
    d_uniqueKey = QUuid::createUuid().toString();

    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }

    {
        auto domNode = domEle.firstChild();
        while (!domNode.isNull())
        {
            auto domEle = domNode.toElement();
            if (domEle.isNull())
            {
                continue;
            }

            if (domEle.tagName() == "MainInfo")
            {
                parseMainInfoNode(domEle, d_SumInfo.mainInfo);
            }
            if (domEle.tagName() == "TargetInfo")
            {
                parseTargetInfoNode(domEle, d_SumInfo.targetInfo);
            }
            if (domEle.tagName() == "ParamInfo")
            {
                parseParamInfoNode(domEle, d_SumInfo.paramInfo);
            }
            domNode = domNode.nextSibling();
        }
    }
}

void ComplexNumberStatusLabel::parseMainInfoNode(const QDomNode& domEle, ComplexNumberStatusLabel::MainInfo& info)
{
    parseInfoNode(domEle, info.mainInfo);
    info.value = domEle.toElement().attribute("value");
}

void ComplexNumberStatusLabel::parseTargetInfoNode(const QDomNode& domEle, ComplexNumberStatusLabel::Info& info) { parseInfoNode(domEle, info); }

void ComplexNumberStatusLabel::parseParamInfoNode(const QDomNode& domEle, ComplexNumberStatusLabel::ParamInfo& info)
{
    parseInfoNode(domEle, info.paramInfo);
    auto elem = domEle.toElement();
    info.isMulti = false;
    if (elem.hasAttribute("isMulti"))
    {
        if (elem.attribute("isMulti").toLower() == "true")
        {
            info.isMulti = true;
        }
    }
}

void ComplexNumberStatusLabel::parseInfoNode(const QDomNode& domEle, ComplexNumberStatusLabel::Info& info)
{
    auto elem = domEle.toElement();
    int iDeviceId = -1;
    d_deviceId >> iDeviceId;
    info.deviceId = elem.attribute("deviceId", QString::number(iDeviceId, 16)).toInt(nullptr, 16);
    info.modeId = elem.attribute("modeId", QString::number(d_modeId, 16)).toInt(nullptr, 16);
    info.unitId = elem.attribute("unitId", "-1").toInt(nullptr, 16);
    info.targetId = elem.attribute("targetId", "-1").toInt(nullptr, 16);
    info.paramId = elem.attribute("paramId", "");
    info.isValid = true;
    if (elem.hasAttribute("isValid"))
    {
        if (elem.attribute("isValid").toLower() == "true")
        {
            info.isValid = true;
        }
        else
        {
            info.isValid = false;
        }
    }
}
