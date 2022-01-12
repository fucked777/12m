#include "NumberStatusFreqLabel.h"
#include <QUuid>

NumberStatusFreqLabel::NumberStatusFreqLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void NumberStatusFreqLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f').replace(rx, "");
    setText(valueString);
    setStatus(Normal);
}

QVariant NumberStatusFreqLabel::getValue() const { return QVariant(); }

void NumberStatusFreqLabel::setDealType(const QString& dealType) { d_dealType = dealType; }

QString NumberStatusFreqLabel::getDealType() const { return d_dealType; }

void NumberStatusFreqLabel::setUniqueKey(const QString& uniqueKey) { d_uniqueKey = uniqueKey; }

QString NumberStatusFreqLabel::getUniqueKey() const { return d_uniqueKey; }

void NumberStatusFreqLabel::setDevids(QVariantList& list) { d_deviceIds = list; }

QVariantList NumberStatusFreqLabel::getDevids() { return d_deviceIds; }

void NumberStatusFreqLabel::setIsMultiTarget(const bool isMultiTarget) { d_isMultiTarget = isMultiTarget; }

bool NumberStatusFreqLabel::getIsMultiTarget() const { return d_isMultiTarget; }

QList<NumberStatusFreqLabel::SumInfo> NumberStatusFreqLabel::getParamInfos() { return d_SumInfos; }

QMap<int, QList<NumberStatusFreqLabel::MultiTargetSumInfo>> NumberStatusFreqLabel::getMultiTargetParamInfos() { return d_MultiTargetSumInfos; }

void NumberStatusFreqLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    d_dealType = "All_Freq_Number_Status";
    d_uniqueKey = QUuid::createUuid().toString();

    QString devids = domEle.attribute("devids");
    for (QString& devid : devids.split(";"))
    {
        int iDevid = devid.toInt(nullptr, 16);
        d_deviceIds.append(iDevid);
    }

    if (domEle.attribute("isMultiTarget") == "true")
    {
        d_isMultiTarget = true;

        auto domNode = domEle.firstChild();
        while (!domNode.isNull())
        {
            auto domEle = domNode.toElement();
            if (domEle.isNull())
            {
                continue;
            }

            int modeId = -1;
            QList<MultiTargetSumInfo> multiTargetSumInfos;
            parseModeNode(domEle, modeId, multiTargetSumInfos);
            d_MultiTargetSumInfos.insert(modeId, multiTargetSumInfos);
            domNode = domNode.nextSibling();
        }
    }
    else
    {
        d_isMultiTarget = false;

        auto domNode = domEle.firstChild();
        while (!domNode.isNull())
        {
            auto domEle = domNode.toElement();
            if (domEle.isNull())
            {
                continue;
            }

            SumInfo sumInfo;
            parseItemNode(domEle, sumInfo);
            d_SumInfos.append(sumInfo);
            domNode = domNode.nextSibling();
        }
    }
}

void NumberStatusFreqLabel::parseModeNode(const QDomElement& domEle, int& modeId, QList<MultiTargetSumInfo>& infos)
{
    modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            continue;
        }

        MultiTargetSumInfo info;
        parseItemNode(domEle, info);
        infos.append(info);
        domNode = domNode.nextSibling();
    }
}

void NumberStatusFreqLabel::parseItemNode(const QDomElement& domEle, NumberStatusFreqLabel::SumInfo& info)
{
    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            continue;
        }

        if (domEle.tagName() == "ParamInfo")
        {
            ParamInfo paramInfo;
            parseParamInfoNode(domEle, paramInfo);
            info.paramInfo = paramInfo;
        }
        if (domEle.tagName() == "MainInfo")
        {
            MainInfo mainInfo;
            parseMainInfoNode(domEle, mainInfo);
            info.mainInfo = mainInfo;
        }
        domNode = domNode.nextSibling();
    }
}

void NumberStatusFreqLabel::parseItemNode(const QDomElement& domEle, MultiTargetSumInfo& info)
{
    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            continue;
        }

        if (domEle.tagName() == "ParamInfo")
        {
            ParamInfo paramInfo;
            parseParamInfoNode(domEle, paramInfo);
            info.paramInfo = paramInfo;
        }
        if (domEle.tagName() == "MainInfo")
        {
            MainInfo mainInfo;
            parseMainInfoNode(domEle, mainInfo);
            info.mainInfo = mainInfo;
        }
        if (domEle.tagName() == "TargetInfo")
        {
            TargetInfo targetInfo;
            parseTargetInfoNode(domEle, targetInfo);
            info.targetInfo = targetInfo;
        }
        domNode = domNode.nextSibling();
    }
}

void NumberStatusFreqLabel::parseParamInfoNode(const QDomElement& domEle, NumberStatusFreqLabel::ParamInfo& info)
{
    int iDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    DeviceID id;
    id << iDeviceId;
    info.deviceId = id;
    info.modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    info.unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    info.paramId = domEle.attribute("paramId");
}

void NumberStatusFreqLabel::parseMainInfoNode(const QDomElement& domEle, NumberStatusFreqLabel::MainInfo& info)
{
    int iDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    DeviceID id;
    id << iDeviceId;
    info.deviceId = id;
    info.modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    info.unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    info.paramId = domEle.attribute("paramId");
    info.value = domEle.attribute("value");
}

void NumberStatusFreqLabel::parseTargetInfoNode(const QDomElement& domEle, NumberStatusFreqLabel::TargetInfo& info)
{
    int iDeviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    DeviceID id;
    id << iDeviceId;
    info.deviceId = id;
    info.modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    info.unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    info.paramId = domEle.attribute("paramId");
    QString type = domEle.attribute("type");
    if (type == "Multi")
    {
        info.type = Multi_Type;
    }
    else if (type == "K2")
    {
        info.type = K2_Type;
    }
    else
    {
        info.type = Single_Type;
    }
}
