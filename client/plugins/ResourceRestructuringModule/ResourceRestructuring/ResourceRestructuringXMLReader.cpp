#include "ResourceRestructuringXMLReader.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>

#include "PlatformConfigTools.h"

ResourceRestructuringXMLReader::ResourceRestructuringXMLReader() {}

QList<BBEConfig> ResourceRestructuringXMLReader::getBBEConfig()
{
    QString path = PlatformConfigTools::configBusiness("ResourceRestructuringManager/BBEConfig.xml");

    QList<BBEConfig> workSystems;
    parseResourceRestructingFile(path, workSystems);

    return workSystems;
}

QMap<int, QString> ResourceRestructuringXMLReader::getBBEIDAndType() { return d_ID_Type; }

void ResourceRestructuringXMLReader::parseResourceRestructingFile(const QString& filePath, QList<BBEConfig>& workSystems)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "配置基带配置文件读取错误";
        return;
    }

    QString errorMsg;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg))
    {
        file.close();
        return;
    }

    file.close();

    QDomNode workSysNode = dom.documentElement().firstChild();
    while (!workSysNode.isNull())
    {
        QDomElement workSysElement = workSysNode.toElement();
        if (workSysElement.nodeName() == "BBEConfig")
        {
            BBEConfig workSystem;

            workSystem.id = workSysElement.attribute("id").toInt();
            workSystem.desc = workSysElement.attribute("desc").trimmed();

            parseDevicesTypeNode(workSysNode, workSystem.devicesTypes);

            workSystems.append(workSystem);
        }
        workSysNode = workSysNode.nextSibling();
    }
}

void ResourceRestructuringXMLReader::parseDevicesTypeNode(const QDomNode& workSysNode, QList<DevicesType>& devicesTypes)
{
    QDomNode child = workSysNode.firstChild();
    while (!child.isNull())
    {
        QDomElement childElement = child.toElement();

        if (childElement.nodeName() == "DevicesType")
        {
            DevicesType item;
            item.id = childElement.attribute("id").toInt(nullptr, 16);
            item.desc = childElement.attribute("desc").trimmed();

            parseDeviceTypeNode(childElement, item.deviceTypes);
            devicesTypes.append(item);
        }
        child = child.nextSibling();
    }
}

void ResourceRestructuringXMLReader::parseDeviceTypeNode(const QDomNode& workSysNode, QList<DeviceType>& deviceTypes)
{
    QDomNode child = workSysNode.firstChild();
    while (!child.isNull())
    {
        QDomElement childElement = child.toElement();

        if (childElement.nodeName() == "DeviceType")
        {
            DeviceType item;
            item.id = childElement.attribute("id").toInt(nullptr, 16);
            item.desc = childElement.attribute("desc").trimmed();

            parseDevNode(childElement, item.devs);
            deviceTypes.append(item);
        }
        child = child.nextSibling();
    }
}

void ResourceRestructuringXMLReader::parseDevNode(const QDomNode& deviceTypeNode, QList<Dev>& devs)
{
    QDomNode child = deviceTypeNode.firstChild();
    while (!child.isNull())
    {
        QDomElement childElement = child.toElement();

        if (childElement.nodeName() == "Dev")
        {
            Dev item;
            item.id = childElement.attribute("id").toInt(nullptr, 16);
            item.desc = childElement.attribute("desc").trimmed();
            item.deviceType = deviceTypeNode.toElement().attribute("desc").trimmed();

            parseEntryNode(childElement, item.enums);
            d_ID_Type[item.id] = item.deviceType;
            devs.append(item);
        }
        child = child.nextSibling();
    }
}

void ResourceRestructuringXMLReader::parseEntryNode(const QDomNode& devNode, QMap<int, QString>& paramEnums)
{
    auto child = devNode.firstChild();
    while (!child.isNull())
    {
        auto childElement = child.toElement();
        if (childElement.nodeName() == "Enum")
        {
            auto desc = childElement.attribute("desc");                         // 枚举值描述
            int value = childElement.attribute("deviceId").toInt(nullptr, 16);  // 枚举值

            paramEnums[value] = desc;
        }
        child = child.nextSibling();
    }
}
