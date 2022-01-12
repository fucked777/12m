#include "ComplexLEDStatusLabel.h"
#include <QHBoxLayout>
#include <QUuid>

static constexpr const char* redQss = "QLabel {"
                                      "background-image: url(:/image/red.png);"
                                      "background-position:center;"
                                      "background-repeat:no-repeat;"
                                      "border-style: none;"
                                      "}";

static constexpr const char* greenQss = "QLabel {"
                                        "background-image: url(:/image/green.png);"
                                        "background-position:center;"
                                        "background-repeat:no-repeat;"
                                        "border-style:none;"
                                        "}";

static constexpr const char* grayQss = "QLabel {"
                                       "background-image: url(:/image/gray.png);"
                                       "background-position:center;"
                                       "background-repeat:no-repeat;"
                                       "border-style:none;"
                                       "}";

ComplexLEDStatusLabel::ComplexLEDStatusLabel(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : BaseWidget(domEle, parent)
    , StatusController()
    , d_dealType("")
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initLayout();

    setMinimumSize(28, 28);
    setStyleSheet(grayQss);

    parseNode(domEle);
}

void ComplexLEDStatusLabel::setValue(const QVariant& value)
{
    auto ledItem = d_sumInfo.ledInfo[value];
    setStatus(ledItem.status);
    setToolTip(ledItem.toolTip);
}

QVariant ComplexLEDStatusLabel::getValue() const { return QVariant(); }

void ComplexLEDStatusLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString ComplexLEDStatusLabel::getDealType() const { return d_dealType; }

void ComplexLEDStatusLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString ComplexLEDStatusLabel::getUniqueKey() const { return d_uniqueKey; }

void ComplexLEDStatusLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList ComplexLEDStatusLabel::getDevids() { return d_deviceIds; }

ComplexLEDStatusLabel::SumInfo& ComplexLEDStatusLabel::getSumInfo() { return d_sumInfo; }

void ComplexLEDStatusLabel::statusChanged(StatusController::Status status)
{
    switch (status)
    {
    case Unknown:
    {
        setStyleSheet(grayQss);
    }
    break;
    case Normal: setStyleSheet(greenQss); break;
    case Abnormal: setStyleSheet(redQss); break;
    default: setStyleSheet(grayQss); break;
    }
}

void ComplexLEDStatusLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void ComplexLEDStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = "All_Complex_LED_Status";
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
        if (subDomEle.tagName() == "MainInfo")
        {
            parseMainInfo(subDomEle, d_sumInfo.mainInfo);
        }
        if (subDomEle.tagName() == "TargetInfo")
        {
            parseTargetInfo(subDomEle, d_sumInfo.targetInfo);
        }
        if (subDomEle.tagName() == "ParamInfo")
        {
            parseParamInfo(subDomEle, d_sumInfo.paramInfo);
        }
        if (subDomEle.tagName() == "LedInfo")
        {
            parseLedInfo(subDomEle, d_sumInfo.ledInfo);
        }
        domNode = domNode.nextSibling();
    }
}

void ComplexLEDStatusLabel::parseMainInfo(const QDomElement& domEle, ComplexLEDStatusLabel::MainInfo& mainInfo)
{
    parseInfo(domEle, mainInfo.mainInfo);
    mainInfo.value = domEle.attribute("value");
}

void ComplexLEDStatusLabel::parseTargetInfo(const QDomElement& domEle, ComplexLEDStatusLabel::TargetInfo& targetInfo)
{
    parseInfo(domEle, targetInfo.targetInfo);
}

void ComplexLEDStatusLabel::parseParamInfo(const QDomElement& domEle, ComplexLEDStatusLabel::ParamInfo& paramInfo)
{
    parseInfo(domEle, paramInfo.paramInfo);
    auto elem = domEle.toElement();
    paramInfo.isMulti = false;
    if (elem.hasAttribute("isMulti"))
    {
        if (elem.attribute("isMulti").toLower() == "true")
        {
            paramInfo.isMulti = true;
        }
    }
}

void ComplexLEDStatusLabel::parseLedInfo(const QDomElement& domEle, QMap<QVariant, ComplexLEDStatusLabel::LEDItem>& ledInfo)
{
    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        auto tagName = subDomEle.tagName();
        if (tagName == "Item")
        {
            LEDItem ledItem;
            ledItem.value = subDomEle.attribute("value");
            QString color = subDomEle.attribute("ledColor");
            if (color.toLower() == "green")
            {
                ledItem.status = Normal;
            }
            else if (color.toLower() == "red")
            {
                ledItem.status = Abnormal;
            }
            else
            {
                ledItem.status = Unknown;
            }
            ledItem.toolTip = subDomEle.attribute("toolTip");
            ledInfo.insert(ledItem.value, ledItem);
        }
        domNode = domNode.nextSibling();
    }
}

void ComplexLEDStatusLabel::parseInfo(const QDomElement& domEle, ComplexLEDStatusLabel::Info& info)
{
    auto elem = domEle.toElement();
    int iDeviceId = -1;
    d_deviceId >> iDeviceId;
    info.deviceId = /*elem.attribute("deviceId", QString::number(iDeviceId, 16)).toInt(nullptr, 16)*/ d_deviceId;
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
