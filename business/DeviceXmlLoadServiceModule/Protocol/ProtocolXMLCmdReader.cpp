#include "ProtocolXMLCmdReader.h"
#include "ProtocolXmlTypeHelper.h"

#include <QDebug>

ProtocolXMLCmdReader::ProtocolXMLCmdReader() {}

QMap<QString, CmdXMLData> ProtocolXMLCmdReader::loadCmdProtocolXml(QString filePath)
{
    QDir dir(filePath);

    QMap<QString, CmdXMLData> ret_Map;

    if (!dir.exists())
    {
        return ret_Map;
    }
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    auto dirInfoList = dir.entryInfoList();
    ///遍历文件夹
    for (auto& info : dirInfoList)
    {
        auto dirPath = info.filePath();
        auto fileName = info.fileName();
        auto sysName = fileName.split("_").at(1);
        dir.setPath(dirPath);
        if (!dir.exists())
        {
            continue;
        }

        QFileInfoList fileInfoList;
        loadProtocolXml(dirPath, fileInfoList);
        CmdXMLData cmdXmlData;
        ///遍历单元属性文件夹
        for (auto& fileInfo : fileInfoList)
        {
            auto childPath = fileInfo.filePath();
            auto childfileName = fileInfo.fileName();
            if (childfileName.contains("Enum"))
            {
                openAndParseCmdEnumXml(childPath, cmdXmlData.cmdEnumMap);
            }
            else
            {
                openAndParseCmdProtocolXML(childPath, cmdXmlData.cmdAttrMap);
            }
        }
        ret_Map.insert(sysName, cmdXmlData);
    }

    return ret_Map;
}

void ProtocolXMLCmdReader::loadProtocolXml(const QString& filePath, QFileInfoList& infoList)
{
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir())
    {
        QDir dir(filePath);
        dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

        for (const QFileInfo& info : dir.entryInfoList())
        {
            loadProtocolXml(info.absoluteFilePath(), infoList);
        }
    }
    else if (fileInfo.isFile())
    {
        infoList.append(fileInfo);
    }
}

void ProtocolXMLCmdReader::openAndParseCmdEnumXml(QString path, CmdEnumMap& enumMap)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();
    QString extenType = root.attribute("ExtenType").trimmed();

    int modeType = root.attribute("ModeType").trimmed().toInt(0, 16);
    QString parentName;

    QMap<QString, DevEnum> enumFile;

    /// parse xml
    parseEnumXML(&root, enumFile, parentName);

    if (enumMap.contains(extenType))
    {
        enumMap[extenType].insert(modeType, enumFile);
    }
    else
    {
        QMap<int, QMap<QString, DevEnum>> modeEnumMap;
        modeEnumMap.insert(modeType, enumFile);
        enumMap.insert(extenType, modeEnumMap);
    }

    file.close();
}

void ProtocolXMLCmdReader::parseEnumXML(QDomElement* docElem, QMap<QString, DevEnum>& ret_Map, QString& parentName)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "enum")
            {
                DevEnum devEnum;
                devEnum.desc = element.attribute("desc");
                devEnum.name = element.attribute("name");

                parentName = devEnum.name;

                ret_Map.insert(parentName, devEnum);
            }
            else if (tagName == "entry")
            {
                DevEnumEntry devEntry;
                devEntry.desc = element.attribute("desc");
                devEntry.uValue = element.attribute("uvalue");
                devEntry.sValue = element.attribute("svalue");

                ret_Map[parentName].emumEntryList.push_back(devEntry);
            }
            parseEnumXML(&element, ret_Map, parentName);
        }
        node = node.nextSibling();
    }
}

bool ProtocolXMLCmdReader::openAndParseCmdProtocolXML(QString path, CmdAttrMap& ret_Map)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    QDomElement root = doc.documentElement();

    CmdAttribute cmdAttribute;
    QString str_Id = root.attribute("cmdID").trimmed();
    QString extenType = root.attribute("ExtenType").trimmed();
    int modeType = root.attribute("ModeType").trimmed().toInt(0, 16);
    QString isShow = root.attribute("isShow").trimmed();
    if (!isShow.isEmpty() || isShow != nullptr)
    {
        cmdAttribute.isShow = isShow.toInt();
    }

    cmdAttribute.desc = root.attribute("desc").trimmed();
    cmdAttribute.extenType = extenType;

    /// parse xml
    QString parent = "";
    int recycleId = 0;
    //处理布局和分组
    QMap<QString, QList<ParameterAttribute>> statusGroupMap;                   //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiStatusGroupMap;   //多目标状态分组
    QMap<QString, QList<ParameterAttribute>> settingGroupMap;                  //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiSettingGroupMap;  //多目标设置参数分组
    QMap<QString, UnitLayout> unitLayoutMap;
    QMap<int, UnitMultiTarget> targetMap;  //每个目标的简要描述
    parseCmdProtocolXML(&root, cmdAttribute, parent, recycleId, targetMap, statusGroupMap, multiStatusGroupMap, settingGroupMap, multiSettingGroupMap,
                        unitLayoutMap);
    int id = str_Id.toInt();

    cmdAttribute.cmdNumb = id;

    ///分机是存在
    if (ret_Map.contains(extenType))
    {
        //这个模式已经存在
        if (ret_Map[extenType].contains(modeType))
        {
            ret_Map[extenType][modeType].insert(id, cmdAttribute);
        }
        //这个模式不存在 创建模式
        else
        {
            QMap<int, CmdAttribute> cmdMap;
            cmdMap.insert(id, cmdAttribute);
            ret_Map[extenType].insert(modeType, cmdMap);
        }
    }
    //分机不存在
    else
    {
        QMap<int, CmdAttribute> cmdMap;
        cmdMap.insert(id, cmdAttribute);
        QMap<int, QMap<int, CmdAttribute>> modeMap;
        modeMap.insert(modeType, cmdMap);
        ret_Map.insert(extenType, modeMap);
    }

    file.close();

    return true;
}

void ProtocolXMLCmdReader::parseCmdProtocolXML(QDomElement* docElem, CmdAttribute& cmdAttribute, QString& parentName, int& recycleId,
                                               QMap<int, UnitMultiTarget>& targetMap,
                                               QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                  //普通状态分组
                                               QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,  //多目标状态分组
                                               QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                 //普通设置参数分组
                                               QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap,  //多目标设置参数分组
                                               QMap<QString, UnitLayout>& unitLayoutMap)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "request")
            {
                parentName = tagName;
                recycleId = -1;
            }
            else if (tagName == "result")
            {
                parentName = tagName;
                recycleId = -1;
            }
            else if (tagName == "recycle")
            {
                recycleId = element.attribute("id").toInt();
            }
            else if (tagName == "field")
            {
                ParameterAttribute cmdBase;

                parseParameterAttribute(cmdBase, element, targetMap, statusGroupMap, multiStatusGroupMap, settingGroupMap, multiSettingGroupMap);
                if (parentName == "request")
                {
                    if (recycleId > 0)
                    {
                        if (cmdAttribute.requestRecycle.contains(recycleId))
                        {
                            cmdAttribute.requestRecycle[recycleId].push_back(cmdBase);
                            cmdAttribute.multiSettingGroupMap = multiSettingGroupMap;
                            cmdAttribute.targetData = targetMap;
                        }
                        else
                        {
                            QList<ParameterAttribute> list;
                            list.push_back(cmdBase);
                            cmdAttribute.requestRecycle.insert(recycleId, list);
                            cmdAttribute.multiSettingGroupMap = multiSettingGroupMap;
                            cmdAttribute.targetData = targetMap;
                        }
                    }
                    else
                    {
                        cmdAttribute.requestList.push_back(cmdBase);
                        cmdAttribute.settingGroupMap = settingGroupMap;
                        cmdAttribute.targetData = targetMap;
                    }
                }
                else if (parentName == "result")
                {
                    if (recycleId > 0)
                    {
                        if (cmdAttribute.resultRecycle.contains(recycleId))
                        {
                            cmdAttribute.resultRecycle[recycleId].push_back(cmdBase);
                            cmdAttribute.multiStatusGroupMap = multiStatusGroupMap;
                            cmdAttribute.targetData = targetMap;
                        }
                        else
                        {
                            QList<ParameterAttribute> list;
                            list.push_back(cmdBase);
                            cmdAttribute.resultRecycle.insert(recycleId, list);
                            cmdAttribute.multiStatusGroupMap = multiStatusGroupMap;
                            cmdAttribute.targetData = targetMap;
                        }
                    }
                    else
                    {
                        cmdAttribute.resultList.push_back(cmdBase);
                        cmdAttribute.statusGroupMap = statusGroupMap;
                        cmdAttribute.targetData = targetMap;
                    }
                }
            }
            else if (tagName == "layout")
            {
                QDomNode layoutNode = element.firstChild();
                while (!layoutNode.isNull())
                {
                    QDomElement layoutElement = layoutNode.toElement();
                    if (!layoutElement.isNull())
                    {
                        QString tempTagName = layoutElement.tagName();
                        if (tempTagName == "item")
                        {
                            UnitLayout unitLayout;
                            unitLayout.id = layoutElement.attribute("id");
                            unitLayout.desc = layoutElement.attribute("desc");
                            unitLayout.row = layoutElement.attribute("row").toInt();
                            unitLayout.col = layoutElement.attribute("col").toInt();
                            unitLayout.rowspan = layoutElement.attribute("rowspan").toInt();
                            unitLayout.colspan = layoutElement.attribute("colspan").toInt();
                            if (unitLayoutMap.contains(unitLayout.id))
                            {
                                qDebug() << "界面布局配置重复";
                            }
                            else
                            {
                                unitLayoutMap[unitLayout.id] = unitLayout;
                                cmdAttribute.unitLayoutMap[unitLayout.id] = unitLayout;
                            }
                        }
                    }
                    layoutNode = layoutNode.nextSibling();
                }
            }
            parseCmdProtocolXML(&element, cmdAttribute, parentName, recycleId, targetMap, statusGroupMap, multiStatusGroupMap, settingGroupMap,
                                multiSettingGroupMap, unitLayoutMap);
        }
        node = node.nextSibling();
    }
}

void ProtocolXMLCmdReader::parseParameterAttribute(ParameterAttribute& base, const QDomElement& element, QMap<int, UnitMultiTarget>& targetMap,
                                                   QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                  //普通状态分组
                                                   QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,  //多目标状态分组
                                                   QMap<QString, QList<ParameterAttribute>>& settingGroupMap,  //普通设置参数分组
                                                   QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap  //多目标设置参数分组
)
{
    QString pid = element.attribute("pid").trimmed();
    if (pid != nullptr || !pid.isEmpty())
    {
        base.pid = pid.toInt();
    }

    base.id = element.attribute("id").trimmed();
    base.desc = element.attribute("desc").trimmed();

    QString dataType = element.attribute("dataType").trimmed();
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

    QString displayFormat = element.attribute("displayFormat").trimmed();
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

    base.minValue = element.attribute("minValue").trimmed();
    base.maxValue = element.attribute("maxValue").trimmed();
    if (element.hasAttribute("fixValue"))
    {
        base.fixValue = QVariant(element.attribute("fixValue").trimmed());
    }

    QString step = element.attribute("step").trimmed();
    if (!step.isEmpty())
        base.step = step;

    QString unit = element.attribute("unit").trimmed();
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

    if (element.hasAttribute("initValue"))
    {
        base.initValue = QVariant(element.attribute("initValue").trimmed());
    }

    QString isShow = element.attribute("isShow").trimmed();
    if (!isShow.isEmpty() || isShow != nullptr)
    {
        base.isShow = isShow.toInt();
    }
    QString isEnable = element.attribute("isEnable").trimmed();
    if (!isEnable.isEmpty() || isEnable != NULL)
    {
        base.isEnable = isEnable.toInt();
    }

    QString groupId = element.attribute("GroupId").trimmed();
    if (groupId == "")
    {
        groupId = "undetermined";
    }
    base.groupId = groupId;

    base.formula = element.attribute("formula").trimmed();

    UnitMultiTarget targetData;
    auto paranNodeName = element.parentNode().nodeName();
    targetData.id = element.parentNode().toElement().attribute("id").toInt();
    targetData.recycleNum = element.parentNode().toElement().attribute("recycleNum");
    targetData.desc = element.parentNode().toElement().attribute("desc");
    if (paranNodeName == "recycle")
    {
        if (base.pid >= 100)
        {
            multiSettingGroupMap[targetData.id][groupId].append(base);
            targetMap[targetData.id] = targetData;
        }
        else
        {
            multiStatusGroupMap[targetData.id][groupId].append(base);
            targetMap[targetData.id] = targetData;
        }
    }
    else
    {
        if (base.pid >= 100)
        {
            settingGroupMap[groupId].append(base);
        }
        else
        {
            statusGroupMap[groupId].append(base);
        }
    }
}
