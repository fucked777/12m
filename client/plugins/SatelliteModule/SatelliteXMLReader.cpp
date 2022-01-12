#include "SatelliteXMLReader.h"
#include "DevProtocol.h"
#include "PlatformInterface.h"
#include "ProtocolXmlTypeHelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QMap>
#include <QMessageBox>
#include <QString>

SalliteModeXMLReader::SalliteModeXMLReader() {}

void SalliteModeXMLReader::loadConfigMacroWorksystemXML(QList<SalliteModeInfo>& map, int& maxWorkMode, int& stationBz)
{
    QString path = PlatformConfigTools::configBusiness("SatelliteManager/SatelliteManager.xml");

    QDomElement root = GetFileDomElement(path);
    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement salliteElement = configNode.toElement();
        if (salliteElement.nodeName() == "Worksystem")
        {
            SalliteModeInfo salliteModeInfo;
            salliteModeInfo.m_modeID = salliteElement.attribute("id").trimmed().toInt();  //模式ID
            salliteModeInfo.m_modeName = salliteElement.attribute("desc").trimmed();      //模式名称
            auto xmlPath = salliteElement.attribute("xmlPath").trimmed();

            loadSalliteXmlPath(salliteModeInfo, xmlPath);
            loadSalliteEnumXmlPath(salliteModeInfo, xmlPath);

            map.append(salliteModeInfo);
        }
        else if (salliteElement.nodeName() == "MaxWorkMode")
        {
            maxWorkMode = salliteElement.attribute("value").trimmed().toInt();  //最大的模式组合
        }
        else if (salliteElement.nodeName() == "StationBZ")
        {
            stationBz = salliteElement.attribute("value").trimmed().toInt();  //最大的模式组合
        }
        configNode = configNode.nextSibling();
    }
}

void SalliteModeXMLReader::loadSalliteXmlPath(SalliteModeInfo& data, const QString& xmlPath)
{
    QString path = PlatformConfigTools::configBusiness("SatelliteManager/" + xmlPath);
    QFileInfo fileInfo(path);
    if (!fileInfo.isFile())
    {
        return;
    }

    QDomElement root = GetFileDomElement(path);

    QDomNode configNode = root.firstChild();
    while (!configNode.isNull())
    {
        QDomElement unitElement = configNode.toElement();
        if (unitElement.nodeName() == "unit")
        {
            SalliteModeInfoItem salliteModeInfoItem;

            QList<ParameterAttribute> m_paramList;

            salliteModeInfoItem.m_groupName = unitElement.attribute("groupName").trimmed();    //分组名称
            salliteModeInfoItem.m_deviceID = unitElement.attribute("deviceID").trimmed();      //设备分机信息
            salliteModeInfoItem.m_unitID = unitElement.attribute("unitID").trimmed().toInt();  //参数ID

            parseParameterAttribute(salliteModeInfoItem.m_paramList, unitElement);
            data.m_modeParamList.append(salliteModeInfoItem);
        }
        configNode = configNode.nextSibling();
    }
}

void SalliteModeXMLReader::loadSalliteEnumXmlPath(SalliteModeInfo& data, const QString& xmlPath)
{
    QString path = PlatformConfigTools::configBusiness("SatelliteManager/" + QString("Enum_") + xmlPath);
    QFileInfo fileInfo(path);
    if (!fileInfo.isFile())
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
                data.m_enumMap[devEnum.name] = devEnum;
            }
        }
        configNode = configNode.nextSibling();
    }
}

void SalliteModeXMLReader::parseEntry(QList<DevEnumEntry>& list, const QDomNode& docElem)
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

void SalliteModeXMLReader::loadJsonComand(const QString& filePath, QMap<QString, QString>) {}

QDomElement SalliteModeXMLReader::GetFileDomElement(QString& filePath)
{
    QDomDocument document;

    QDomElement root;

    QFile file(filePath);

    QString errorMessage;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("卫星管理文件打开失败:%1,%2").arg(filePath).arg(file.errorString()), QString("确定"));
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

void SalliteModeXMLReader::parseParameterAttribute(QList<ParameterAttribute>& list, const QDomNode& element)
{
    QDomNode child = element.firstChild();
    while (!child.isNull())
    {
        ParameterAttribute base;

        QDomElement childElement = child.toElement();

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
        list.append(base);
        child = child.nextSibling();
    }
}
