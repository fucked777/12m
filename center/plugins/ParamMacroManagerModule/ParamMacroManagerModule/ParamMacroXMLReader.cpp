#include "ParamMacroXMLReader.h"
#include "DevProtocol.h"
#include "PlatformConfigTools.h"
#include "ProtocolXmlTypeHelper.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QMetaEnum>

ParamMacroXMLReader::ParamMacroXMLReader() {}

void ParamMacroXMLReader::loadParamMacroWorksystemXML(QList<ParamMacroModeInfo>& map)
{
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/ParamMacroManager.xml");
    if (!QFile::exists(path))
    {
        return;
    }
    QDomElement root = GetFileDomElement(path);

    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement paramMacroElement = configNode.toElement();
        if (paramMacroElement.nodeName() == "Worksystem")
        {
            ParamMacroModeInfo paramMacroModeInfo;
            paramMacroModeInfo.m_modeID = (SystemWorkMode)paramMacroElement.attribute("id").trimmed().toInt();  //模式ID
            paramMacroModeInfo.m_modeName = paramMacroElement.attribute("desc").trimmed();                      //模式名称
            auto xmlPath = paramMacroElement.attribute("xmlPath").trimmed();

            loadParamMacroXmlPath(paramMacroModeInfo, xmlPath);
            loadFrequencyEnumXmlPath(paramMacroModeInfo, xmlPath);

            map.append(paramMacroModeInfo);
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadParamMacroXmlPath(ParamMacroModeInfo& data, const QString& xmlPath)
{
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/" + xmlPath);
    if (!QFile::exists(path))
    {
        return;
    }
    QDomElement root = GetFileDomElement(path);
    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "frequency")
        {
            DeviceParamMacroInfoItem deviceParamMacroInfoItem;

            loadFrequencyNode(data.m_frequencyList, unitElement);
        }
        else if (unitElement.nodeName() == "devices")
        {
            loadDeviceItemParamMacroXmlPath(data, unitElement);
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::parseDevice(QList<DeviceParamMacroInfoItem>& list, const QDomNode& docElem)
{
    QDomNode node = docElem.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "device")
            {
                DeviceParamMacroInfoItem devEntry;
                devEntry.m_devName = element.attribute("name");

                list.push_back(devEntry);
            }
        }
        node = node.nextSibling();
    }
}

void ParamMacroXMLReader::loadDeviceItemParamMacroXmlPath(ParamMacroModeInfo& data, const QDomNode& docElem)
{
    QDomNode configNode = docElem.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "device")
        {
            DeviceParamMacroInfoItem item;

            item.m_devName = unitElement.attribute("name").trimmed();         //设备名称
            item.m_stationName = unitElement.attribute("station").trimmed();  //站名称
            item.m_deviceID = unitElement.attribute("deviceID").trimmed();    //当前设备的ID
            auto xmlPath = unitElement.attribute("xmlPath").trimmed();
            loadDeviceItemParamMacroItem(item, xmlPath);
            loadDeviceUnitEnum(item, xmlPath);  //加载这个设备所有的枚举
            data.m_deviceList.append(item);
        }

        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadDeviceItemParamMacroItem(DeviceParamMacroInfoItem& data, const QString& xmlPath)
{
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/" + xmlPath);
    if (!QFile::exists(path))
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("参数宏配置文件打开失败:%1,文件不存在").arg(path), QString("确定"));
        return;
    }
    QDomElement root = GetFileDomElement(path);
    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "unit")
        {
            loadDeviceUnitParamMacro(data, unitElement);
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadDeviceUnitParamMacro(DeviceParamMacroInfoItem& data, const QDomNode& docElem)
{
    QDomNode configNode = docElem.firstChild();
    auto unitID = docElem.toElement().attribute("unitID").toInt();
    data.m_unitNameMap[unitID] = docElem.toElement().attribute("unitName");

    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "field")
        {
            ParameterAttribute attribute;
            parseParameterAttribute(attribute, unitElement);
            data.m_unitConfigMap[unitID].append(attribute);
        }
        else if (unitElement.nodeName() == "target")
        {
            QDomNode targetNode = unitElement.firstChild();
            auto targetID = unitElement.toElement().attribute("id").toInt();
            while (!targetNode.isNull())
            {
                QDomElement targetElement = targetNode.toElement();

                QList<ParameterAttribute> attribute;
                parseParameterAttributeList(attribute, unitElement);
                data.m_unitRecycleConfigMap[unitID][targetID] = attribute;

                targetNode = targetNode.nextSibling();
            }
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadDeviceUnitEnum(DeviceParamMacroInfoItem& data, QString xmlPath)
{
    //在配置加上对应的后缀
    QStringList split = xmlPath.split("/");
    if (split.size() < 1)
    {
        return;
    }
    split[split.size() - 1] = QString("Enum_") + split[split.size() - 1];
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/" + split.join("/"));
    if (!QFile::exists(path))
    {
        return;
    }
    QDomElement root = GetFileDomElement(path);

    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (!unitElement.isNull())
        {
            QString tagName = unitElement.tagName();
            if (tagName == "enum")
            {
                DevEnum devEnum;
                devEnum.desc = unitElement.attribute("desc");
                devEnum.name = unitElement.attribute("name");

                parseEntry(devEnum.emumEntryList, unitElement);
                data.m_unitEnumMap[devEnum.name] = devEnum;
            }
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadFrequencyNode(QList<ParamMacroModeInfoItem>& list, const QDomNode& docElem)
{
    QDomNode configNode = docElem.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "unit")
        {
            ParamMacroModeInfoItem paramMacroModeInfoItem;

            paramMacroModeInfoItem.m_groupName = unitElement.attribute("groupName").trimmed();  // 分组名称
            paramMacroModeInfoItem.m_deviceID = unitElement.attribute("deviceID").trimmed();    // 设备分机信息
            paramMacroModeInfoItem.enumGroupId = unitElement.attribute("enumGroup").trimmed();  // 在配置枚举文件中对枚举进行分组

            parseParameterAttributeList(paramMacroModeInfoItem.m_paramList, unitElement);
            list.append(paramMacroModeInfoItem);
        }
        else if (unitElement.nodeName() == "change")
        {
            //这里对应是可变参数
            ParamMacroModeInfoItem paramMacroModeInfoItem;
            paramMacroModeInfoItem.isDynamicParam = true;
            paramMacroModeInfoItem.m_groupName = unitElement.attribute("groupName").trimmed();  // 分组名称
            paramMacroModeInfoItem.m_deviceID = unitElement.attribute("deviceID").trimmed();    // 设备分机信息
            paramMacroModeInfoItem.enumGroupId = unitElement.attribute("enumGroup").trimmed();  // 在配置枚举文件中对枚举进行分组
            paramMacroModeInfoItem.dynamicGroupId = unitElement.attribute("dynamicGroupId").trimmed();  // 动态参数组id，需要和和卫星管理一致

            parseParameterAttributeList(paramMacroModeInfoItem.m_paramList, unitElement);
            list.append(paramMacroModeInfoItem);
        }
        configNode = configNode.nextSibling();
    }
}

void ParamMacroXMLReader::loadFrequencyEnumXmlPath(ParamMacroModeInfo& data, QString xmlPath)
{
    QStringList split = xmlPath.split("/");
    if (split.size() < 1)
    {
        return;
    }
    split[split.size() - 1] = QString("Enum_") + split[split.size() - 1];
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/" + split.join("/"));
    if (!QFile::exists(path))
    {
        return;
    }
    QDomElement root = GetFileDomElement(path);

    QDomNode enumGroupNode = root.firstChild();
    while (!enumGroupNode.isNull())
    {
        QDomElement enumGroupEle = enumGroupNode.toElement();
        if (!enumGroupEle.isNull())
        {
            QString tagName = enumGroupEle.tagName();

            if (tagName == "enumGroup")
            {
                auto enumGroup = enumGroupEle.attribute("id");
                QDomNode enumNode = enumGroupEle.firstChild();
                while (!enumNode.isNull())
                {
                    QDomElement enumEle = enumNode.toElement();
                    if (!enumEle.isNull())
                    {
                        QString tagName = enumEle.tagName();
                        if (tagName == "enum")
                        {
                            DevEnum devEnum;
                            devEnum.desc = enumEle.attribute("desc");
                            devEnum.name = enumEle.attribute("name");

                            parseEntry(devEnum.emumEntryList, enumEle);

                            data.m_frequencyEnumMap[enumGroup][devEnum.name] = devEnum;
                        }
                    }

                    enumNode = enumNode.nextSibling();
                }
            }
        }
        enumGroupNode = enumGroupNode.nextSibling();
    }
}

void ParamMacroXMLReader::parseEntry(QList<DevEnumEntry>& list, const QDomNode& docElem)
{
    QDomNode node = docElem.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "entry")
            {
                DevEnumEntry devEntry;
                devEntry.desc = element.attribute("desc");
                devEntry.uValue = element.attribute("uvalue");
                devEntry.sValue = element.attribute("svalue");

                list.push_back(devEntry);
            }
        }
        node = node.nextSibling();
    }
}

void ParamMacroXMLReader::parseParameterAttribute(ParameterAttribute& base, const QDomNode& element)
{
    QDomElement childElement = element.toElement();

    QString pid = childElement.attribute("pid").trimmed();
    if (pid != nullptr || !pid.isEmpty())
    {
        base.pid = pid.toInt();
    }

    base.id = childElement.attribute("id").trimmed();
    base.desc = childElement.attribute("desc").trimmed();

    QString dataType = childElement.attribute("dataType").trimmed();
    if (dataType.contains("|"))
    {
        QStringList list = dataType.split("|");

        if (list.count() > 1)
        {
            QStringList typelist = list.at(0).split(":");

            base.attributeType = ProtocolXmlTypeHelper::getAttributeType(typelist.at(0));

            QString length = list.at(1);
            base.length = length.toInt();
        }
    }
    else
    {
        base.attributeType = ProtocolXmlTypeHelper::getAttributeType(dataType);
        base.length = ProtocolXmlTypeHelper::getAttributeTypeByteLength(dataType);  // cyx V14 主要为解决Hex问题
    }

    QString displayFormat = childElement.attribute("displayFormat").trimmed();
    if (displayFormat.contains("|"))
    {
        QStringList list = displayFormat.split("|");
        if (list.count() > 1)
        {
            base.displayFormat = ProtocolXmlTypeHelper::getDisplayFormat(list.at(0));
            base.enumType = list.at(1);
        }
    }
    else
    {
        base.displayFormat = ProtocolXmlTypeHelper::getDisplayFormat(displayFormat);
    }

    base.minValue = childElement.attribute("minValue").trimmed();
    base.maxValue = childElement.attribute("maxValue").trimmed();
    if (childElement.hasAttribute("fixValue"))
    {
        base.fixValue = QVariant(childElement.attribute("fixValue").trimmed());
    }

    QString step = childElement.attribute("step").trimmed();
    if (!step.isEmpty())
        base.step = step;

    QString unit = childElement.attribute("unit").trimmed();
    if (unit.contains("|"))
    {
        QStringList list = unit.split("|");
        if (list.count() > 1)
        {
            base.unit = list.at(1);
        }
    }
    else
    {
        base.unit = unit;
    }

    base.initValue = QVariant(childElement.attribute("initValue").trimmed());

    QString isShow = childElement.attribute("isShow").trimmed();
    if (!isShow.isEmpty() || isShow != nullptr)
    {
        base.isShow = isShow.toInt();
    }
}

void ParamMacroXMLReader::parseParameterAttributeList(QList<ParameterAttribute>& list, const QDomNode& element)
{
    QDomNode child = element.firstChild();
    while (!child.isNull())
    {
        if (child.toElement().tagName() != "field")
        {
            child = child.nextSibling();
            continue;
        }

        QDomElement childElement = child.toElement();

        ParameterAttribute base;
        QString pid = childElement.attribute("pid").trimmed();
        if (pid != nullptr || !pid.isEmpty())
        {
            base.pid = pid.toInt();
        }
        base.id = childElement.attribute("id").trimmed();
        base.desc = childElement.attribute("desc").trimmed();

        QString dataType = childElement.attribute("dataType").trimmed();
        if (dataType.contains("|"))
        {
            QStringList list = dataType.split("|");

            if (list.count() > 1)
            {
                QStringList typelist = list.at(0).split(":");  // 类似：bit:uint|1
                base.attributeType = ProtocolXmlTypeHelper::getAttributeType(typelist.at(0));

                if (typelist.size() > 1)
                {
                    base.transferType = ProtocolXmlTypeHelper::getAttributeType(typelist.at(1));
                }
                else
                {
                    base.transferType = base.attributeType;
                }

                QString length = list.at(1);
                base.length = length.toInt();
            }
        }
        else
        {
            base.attributeType = ProtocolXmlTypeHelper::getAttributeType(dataType);
            base.length = ProtocolXmlTypeHelper::getAttributeTypeByteLength(dataType);
            base.transferType = base.attributeType;
        }

        QString displayFormat = childElement.attribute("displayFormat").trimmed();
        if (displayFormat.contains("|"))
        {
            QStringList list = displayFormat.split("|");
            if (list.count() > 1)
            {
                base.displayFormat = ProtocolXmlTypeHelper::getDisplayFormat(list.at(0));
                base.enumType = list.at(1);
            }
        }
        else
        {
            base.displayFormat = ProtocolXmlTypeHelper::getDisplayFormat(displayFormat);
        }

        base.minValue = childElement.attribute("minValue").trimmed();
        base.maxValue = childElement.attribute("maxValue").trimmed();
        if (childElement.hasAttribute("fixValue"))
        {
            base.fixValue = QVariant(childElement.attribute("fixValue").trimmed());
        }

        QString step = childElement.attribute("step").trimmed();
        if (!step.isEmpty())
            base.step = step;

        QString unit = childElement.attribute("unit").trimmed();
        if (unit.contains("|"))
        {
            QStringList list = unit.split("|");
            if (list.count() > 1)
            {
                base.unit = list.at(1);
            }
        }
        else
        {
            base.unit = unit;
        }

        base.initValue = QVariant(childElement.attribute("initValue").trimmed());

        QString isShow = childElement.attribute("isShow").trimmed();
        if (!isShow.isEmpty() || isShow != NULL)
        {
            base.isShow = isShow.toInt();
        }
        list.append(base);
        child = child.nextSibling();
    }
}

QDomElement ParamMacroXMLReader::GetFileDomElement(QString& filePath)
{
    QDomDocument document;

    QDomElement root;

    QFile file(filePath);
    if (!QFile::exists(filePath))
    {
        return root;
    }
    QString errorMessage;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("参数宏配置文件打开失败:%1,%2").arg(filePath).arg(file.errorString()),
                              QString("确定"));
        return root;
    }
    if (!document.setContent(&file, false, &errorMessage))
    {
        file.close();
        return root;
    }

    file.close();

    root = document.documentElement();

    return root;
}
