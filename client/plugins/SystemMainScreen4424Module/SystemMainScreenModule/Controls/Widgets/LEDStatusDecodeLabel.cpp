#include "LEDStatusDecodeLabel.h"
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

LEDStatusDecodeLabel::LEDStatusDecodeLabel(const QDomElement& domEle, QWidget* parent)
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

void LEDStatusDecodeLabel::setValue(const QVariant& value)
{
    auto ledItem = mLedItemMap[value];
    setStatus(ledItem.status);
    setToolTip(ledItem.toolTip);
}

QVariant LEDStatusDecodeLabel::getValue() const { return QVariant(); }

void LEDStatusDecodeLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString LEDStatusDecodeLabel::getDealType() const { return d_dealType; }

void LEDStatusDecodeLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString LEDStatusDecodeLabel::getUniqueKey() const { return d_uniqueKey; }

void LEDStatusDecodeLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList LEDStatusDecodeLabel::getDevids() { return d_deviceIds; }

QList<LEDStatusDecodeLabel::LEDParam> LEDStatusDecodeLabel::getParamInfos() { return d_LedParams; }

void LEDStatusDecodeLabel::statusChanged(StatusController::Status status)
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

void LEDStatusDecodeLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void LEDStatusDecodeLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
    d_dealType = "All_Decode_Led_Status";
    int iDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    DeviceID id;
    id << iDeviceId;
    d_deviceId = id;
    d_modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    d_unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    d_targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    d_uniqueKey = QUuid::createUuid().toString();
    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }

    int count = 0;
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
            LEDParam ledParam;

            ledParam.deviceId = d_deviceId;
            ledParam.modeId = d_modeId;
            ledParam.unitId = d_unitId;
            ledParam.targetId = d_targetId;
            ledParam.paramId = subDomEle.attribute("paramId");
            ledParam.normalValue = subDomEle.attribute("green");
            ledParam.desc = subDomEle.attribute("desc");

            ledItem.value = count++;
            ledItem.toolTip = ledParam.desc;
            ledItem.status = Normal;
            mLedItemMap.insert(ledItem.value, ledItem);
            d_LedParams.append(ledParam);
        }

        LEDItem defaultRedLedItem;
        defaultRedLedItem.value = -1;
        defaultRedLedItem.status = Abnormal;
        defaultRedLedItem.toolTip = QString("未锁定");
        mLedItemMap.insert(defaultRedLedItem.value, defaultRedLedItem);

        LEDItem defaultGrayItem;
        defaultGrayItem.value = -2;
        defaultGrayItem.status = Unknown;
        defaultGrayItem.toolTip = QString("");
        mLedItemMap.insert(defaultGrayItem.value, defaultGrayItem);

        domNode = domNode.nextSibling();
    }
}
