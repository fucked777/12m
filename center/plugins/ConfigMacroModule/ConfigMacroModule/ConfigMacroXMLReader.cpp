#include "ConfigMacroXMLReader.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include "PlatformInterface.h"

ConfigMacroXMLReader::ConfigMacroXMLReader() {}

bool ConfigMacroXMLReader::getConfigMacroWorkMode(QList<ConfigMacroWorkMode>& configMacroWorkModes, QString& stationName)
{
    QString path = PlatformConfigTools::configBusiness("ConfigMacroManager/ConfigMacroConfig.xml");
    return parseConfigMacroFile(path, configMacroWorkModes, stationName);
}

bool ConfigMacroXMLReader::parseConfigMacroFile(const QString& filePath, QList<ConfigMacroWorkMode>& configMacroWorkModes, QString& stationName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(nullptr, QString("警告"),
                                 QString("配置宏文件打开失败:%1,%2").arg(filePath).arg(file.errorString(), QString("确定")));
        return false;
    }

    QString errorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg, &errorRow, &errorCol))
    {
        file.close();
        QMessageBox::warning(nullptr, QString("警告"),
                             QString("解析XML文件错误:%1,第%3第%4列，%2").arg(filePath).arg(errorRow).arg(errorCol).arg(errorMsg), QString("确定"));
        return false;
    }

    file.close();

    QDomNode workModeNode = dom.documentElement().firstChild();
    while (!workModeNode.isNull())
    {
        QDomElement workModeElement = workModeNode.toElement();
        if (workModeElement.nodeName() == "WorkMode")
        {
            ConfigMacroWorkMode configMacroWorkMode;

            configMacroWorkMode.workMode = (SystemWorkMode)workModeElement.attribute("id").toInt();  // 配置宏的工作模式
            configMacroWorkMode.desc = workModeElement.attribute("desc").trimmed();                  // 当前模式的名称

            parseItemNode(workModeNode, configMacroWorkMode.items);

            configMacroWorkModes.append(configMacroWorkMode);
        }
        else if (workModeElement.nodeName() == "StationName")
        {
            stationName = workModeElement.attribute("name");
        }
        workModeNode = workModeNode.nextSibling();
    }

    return true;
}

void ConfigMacroXMLReader::parseItemNode(const QDomNode& workModeNode, QList<Item>& items)
{
    QDomNode child = workModeNode.firstChild();
    while (!child.isNull())
    {
        QDomElement childElement = child.toElement();

        if (childElement.nodeName() == "Item")
        {
            Item item;
            item.id = childElement.attribute("id");
            item.desc = childElement.attribute("desc").trimmed();
            item.startBit = childElement.attribute("startBit").trimmed().toInt();
            item.endBit = childElement.attribute("endBit").trimmed().toInt();
            item.offset = childElement.attribute("offset").trimmed().toInt();
            item.linkType = (childElement.attribute("desc").trimmed() == "DownAndUpLink" ? Item::DownAndUpLink : Item::OnlyDownLink);
            item.isManualControl = childElement.attribute("isManualControl", "0") == "0" ? false : true;

            parseEnumNode(childElement, item.enums);

            items.append(item);
        }
        child = child.nextSibling();
    }
}

void ConfigMacroXMLReader::parseEnumNode(const QDomNode& itemNode, QList<EnumEntry>& enums)
{
    auto child = itemNode.firstChild();
    while (!child.isNull())
    {
        auto childElement = child.toElement();
        if (childElement.nodeName() == "Enum")
        {
            EnumEntry enumEntry;

            enumEntry.desc = childElement.attribute("desc");                                       // 枚举值描述
            enumEntry.value = childElement.attribute("value");                                     // 枚举值
            enumEntry.deviceId = DeviceID(childElement.attribute("deviceId").toInt(nullptr, 16));  // 对应的设备ID

            enums << enumEntry;
        }
        child = child.nextSibling();
    }
}
