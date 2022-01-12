#include "EnumStatusLabel.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"

EnumStatusLabel::EnumStatusLabel(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : StatusLabel(domEle, parent)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
    , d_enumId("")
{
    parseNode(domEle);
    initMap();
}

void EnumStatusLabel::setValue(const QVariant& value)
{
    if (d_enums.contains(value.toString()))
    {
        QString desc = d_enums[value.toString()];
        (desc == "无此参数") ? (setText(d_defaultDesc)) : (setText(desc));
        setStatus(Normal);
    }
    else
    {
        setText(d_defaultDesc);
        setStatus(Normal);
    }
}

QVariant EnumStatusLabel::getValue() const { return QVariant(); }

int EnumStatusLabel::getDeviceId() const { return d_deviceId; }

int EnumStatusLabel::getModeId() const { return d_modeId; }

void EnumStatusLabel::initMap()
{
    DeviceID id;
    id << d_deviceId;
    Unit unit = GlobalData::getUnit(id.sysID, id.devID, id.extenID, d_modeId, d_unitId);

    DevEnum devEnum;
    if (d_enumId.isEmpty())
        devEnum = unit.enumMap.value(d_paramId);
    else
        devEnum = unit.enumMap.value(d_enumId);

    bool hasDefaultValue = false;

    QList<DevEnumEntry> enumEntryList = devEnum.emumEntryList;
    for (DevEnumEntry& entry : enumEntryList)
    {
        QString desc = entry.desc;
        if (desc.contains(":"))
            desc = desc.split(":").at(0);
        d_enums.insert(entry.uValue, desc);
        if (desc != "无此参数" && !hasDefaultValue)
        {
            d_defaultDesc = desc;
        }
    }

    if (enumEntryList.size())
    {
        d_defaultDesc = enumEntryList.at(0).desc;
    }
    int cd = 2000;
}

void EnumStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    if (domEle.hasAttribute("deviceId"))
        d_deviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    if (domEle.hasAttribute("modeId"))
        d_modeId = domEle.attribute("modeId").toInt(nullptr, 16);

    d_unitId = domEle.attribute("unitId").toInt();
    d_targetId = domEle.attribute("targetId").toInt();
    d_paramId = domEle.attribute("paramId");

    if (domEle.hasAttribute("modeId"))
        d_modeId = domEle.attribute("modeId").toInt(nullptr, 16);
    if (domEle.hasAttribute("enumId"))
        d_enumId = domEle.attribute("enumId");
}
