#include "LEDStatusStimuLabel.h"
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

LEDStatusStimuLabel::LEDStatusStimuLabel(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
    , StatusController()
    , d_dealType("")
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initLayout();

    setMinimumSize(28, 28);
    setStyleSheet(grayQss);
    parseNode(domEle);
}

void LEDStatusStimuLabel::setValue(const QVariant& value)
{
    auto ledItem = mLedItemMap[value.toString()];
    setStatus(ledItem.status);
    setToolTip(ledItem.toolTip);
}

QVariant LEDStatusStimuLabel::getValue() const { return QVariant(); }

void LEDStatusStimuLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString LEDStatusStimuLabel::getDealType() const { return d_dealType; }

void LEDStatusStimuLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString LEDStatusStimuLabel::getUniqueKey() const { return d_uniqueKey; }

void LEDStatusStimuLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList LEDStatusStimuLabel::getDevids() { return d_deviceIds; }

QList<LEDStatusStimuLabel::SumInfo> LEDStatusStimuLabel::getParamInfos() { return d_SumInfos; }

void LEDStatusStimuLabel::statusChanged(StatusController::Status status)
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

void LEDStatusStimuLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void LEDStatusStimuLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
    int iDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);

    d_dealType = "All_Stimu_Number_Status";
    d_uniqueKey = QUuid::createUuid().toString();
    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }

    LEDItem itemRed;
    itemRed.value = "2";
    itemRed.status = Abnormal;
    itemRed.toolTip = "去激励";

    LEDItem itemGreen;
    itemGreen.value = "1";
    itemGreen.status = Normal;
    itemGreen.toolTip = "上激励";

    LEDItem itemGray;
    itemGray.value = "0";
    itemGray.status = Normal;
    itemGray.toolTip = "未知状态";

    mLedItemMap.insert("0", itemGray);
    mLedItemMap.insert("1", itemGreen);
    mLedItemMap.insert("2", itemRed);

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

                QMap<QString, QString> ledInfo;
                QString redUValue, greenUValue, redSValue, greenSValue;
                redUValue = subChildEle.attribute("red");
                redSValue = "2";
                greenUValue = subChildEle.attribute("green");
                greenSValue = "1";
                ledInfo.insert(redUValue, redSValue);
                ledInfo.insert(greenUValue, greenSValue);
                sumInfo.ledInfo = ledInfo;
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
