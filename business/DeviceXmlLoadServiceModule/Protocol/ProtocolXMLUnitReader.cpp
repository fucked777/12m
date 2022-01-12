#include "ProtocolXMLUnitReader.h"
#include "ProtocolXmlTypeHelper.h"

#include <QDebug>

ProtocolXMLUnitReader::ProtocolXMLUnitReader() {}

QMap<QString, UnitXmlDataMap> ProtocolXMLUnitReader::loadUnitProtocolXml(QString filePath)
{
    QDir dir(filePath);
    QMap<QString, UnitXmlDataMap> ret_Map;

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
        UnitXmlDataMap unitXmlDataMap;
        ///遍历单元属性文件夹
        for (auto& fileInfo : fileInfoList)
        {
            QString childPath = fileInfo.filePath();
            QString childfileName = fileInfo.fileName();
            if (childfileName.contains("Enum"))
            {
                openAndParseUnitEnumXml(childPath, unitXmlDataMap);
            }
            else
            {
                openAndParseUnitProtocolXML(childPath, unitXmlDataMap);
            }
        }
        ret_Map.insert(sysName, unitXmlDataMap);
    }
    return ret_Map;
}

void ProtocolXMLUnitReader::loadProtocolXml(const QString& filePath, QFileInfoList& infoList)
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

void ProtocolXMLUnitReader::openAndParseUnitEnumXml(QString path, UnitXmlDataMap& unitXmlDataMap)
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
    QString unitType = root.attribute("UnitType").trimmed();
    int modeType = root.attribute("ModeType").trimmed().toInt(0, 16);
    QString parentName = "";

    QMap<QString, DevEnum> enumFile;

    /// parse xml
    parseEnumXML(&root, enumFile, parentName);

    if (unitXmlDataMap.contains(modeType))
    {
        if (unitXmlDataMap[modeType].contains(unitType))
        {
            unitXmlDataMap[modeType][unitType].enumMap = enumFile;
        }
        else
        {
            Unit unitXmlData;
            unitXmlData.enumMap = enumFile;
            unitXmlDataMap[modeType].insert(unitType, unitXmlData);
        }
    }
    else
    {
        QMap<QString, Unit> unitXMlDataMap;

        Unit unitXmlData;
        unitXmlData.enumMap = enumFile;

        unitXMlDataMap.insert(unitType, unitXmlData);
        unitXmlDataMap.insert(modeType, unitXMlDataMap);
    }

    file.close();
}

void ProtocolXMLUnitReader::parseEnumXML(QDomElement* docElem, QMap<QString, DevEnum>& ret_Map, QString& parentName)
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

bool ProtocolXMLUnitReader::openAndParseUnitProtocolXML(QString path, UnitXmlDataMap& unitXmlDataMap)
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

    QString unitType = root.attribute("UnitType").trimmed();
    int modeType = root.attribute("ModeType").trimmed().toInt(0, 16);
    QString kpValid = root.attribute("kpValid").trimmed();      //扩频4目标才会用到的有效标识
    QString isAutoMap = root.attribute("isAutoMap").trimmed();  //该单元是否是自动映射单元（只有部分设备的遥测模拟源会用到）
    QString isShow = root.attribute("isShow").trimmed();        //该单元是否显示
    QString isModifySetBtn = root.attribute("isModifySetBtn").trimmed();  //该单元是否需要修改设置按钮

    //按pid顺序分组
    QMap<int, ParameterAttribute> newCommonSettingMap;                  //第一个key是pid  按pid排序
    QMap<int, QMap<int, ParameterAttribute>> newCommonMultiSettingMap;  //第一个key是多目标id，第二个key是pid
    QMap<int, ParameterAttribute> newCommonStatusMap;                   //第一个key是pid  按pid排序
    QMap<int, QMap<int, ParameterAttribute>> newCommonMultiStatusMap;   //第一个key是多目标id，第二个key是pid

    //无分组多目标
    QMap<int, QList<ParameterAttribute>> multiStatusMap;   //多目标状态参数  key是多目标ID
    QMap<int, QList<ParameterAttribute>> multiSettingMap;  //多目标设置参数  key是多目标ID
    //有分组多目标
    QMap<QString, QList<ParameterAttribute>> statusGroupMap;                   //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiStatusGroupMap;   //多目标状态分组
    QMap<QString, QList<ParameterAttribute>> settingGroupMap;                  //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiSettingGroupMap;  //多目标设置参数分组
    //布局
    QMap<QString, UnitLayout> unitLayoutMap;  //布局信息
    //所有普通参数和多目标/一体化参数
    QList<ParameterAttribute> queryResult;                 //存放的是所有普通状态和设置参数数据
    QMap<int, QList<ParameterAttribute>> queryRecycleMap;  //存放的是所有多目标或者一体化参数数据

    QMap<int, UnitMultiTarget> targetMap;  //每个目标的简要描述
                                           // key是其id，如TargetNum1;value是描述，如目标1（上面级）遥控单元数据元素
    QStringList order;
    parseNewUnitGroupProtocolXML(&root, unitLayoutMap, targetMap, newCommonSettingMap, newCommonMultiSettingMap, newCommonStatusMap,
                                 newCommonMultiStatusMap, statusGroupMap, multiStatusGroupMap, settingGroupMap, multiSettingGroupMap, order);

    if (order.isEmpty())
    {
        // queryResult存储所有的普通状态设置参数  这里是按pid排序且不是按order排序的情况
        queryResult.append(newCommonStatusMap.values());
        queryResult.append(newCommonSettingMap.values());

        // queryRecycleMap存储所有的多目标或者一体化状态设置参数  这里是按pid排序且不是按order排序的情况
        for (auto key : newCommonMultiStatusMap.keys())
        {
            queryRecycleMap[key].append(newCommonMultiStatusMap[key].values());
            // multiStatusMap存储所有的普通状态参数  这里是按pid排序且不是按order排序的情况
            multiStatusMap[key].append(newCommonMultiStatusMap[key].values());
        }
        for (auto key : newCommonMultiSettingMap.keys())
        {
            for (auto ppItem : newCommonMultiSettingMap[key].values())
            {
                queryRecycleMap[key].append(ppItem);
                // multiSettingMap存储所有的普通状态参数  这里是按pid排序且不是按order排序的情况
                multiSettingMap[key].append(ppItem);
            }
        }
    }
    else
    {
        //下面都是按order排序
        for (auto orderItem : order)
        {
            if (!statusGroupMap.isEmpty())
            {
                if (statusGroupMap.contains(orderItem))
                {
                    for (auto attr : statusGroupMap.value(orderItem))
                    {
                        queryResult.append(attr);
                    }
                }
            }
            if (!settingGroupMap.isEmpty())
            {
                if (settingGroupMap.contains(orderItem))
                {
                    for (auto attr : settingGroupMap.value(orderItem))
                    {
                        queryResult.append(attr);
                    }
                }
            }

            if (!multiStatusGroupMap.isEmpty())
            {
                for (auto recycleKey : multiStatusGroupMap.keys())
                {
                    QMap<QString, QList<ParameterAttribute>> statusMap = multiStatusGroupMap[recycleKey];
                    if (statusMap.contains(orderItem))
                    {
                        auto statusList = statusMap[orderItem];
                        for (auto attr : statusList)
                        {
                            queryRecycleMap[recycleKey].append(attr);
                            multiStatusMap[recycleKey].append(attr);
                        }
                    }
                }
            }
            if (!multiSettingGroupMap.isEmpty())
            {
                for (auto recycleKey : multiSettingGroupMap.keys())
                {
                    QMap<QString, QList<ParameterAttribute>> setMap = multiSettingGroupMap[recycleKey];
                    if (setMap.contains(orderItem))
                    {
                        auto setList = setMap[orderItem];
                        for (auto attr : setList)
                        {
                            queryRecycleMap[recycleKey].append(attr);
                            multiSettingMap[recycleKey].append(attr);
                        }
                    }
                }
            }
        }
    }

    //参数的信息
    unitXmlDataMap[modeType][unitType].queryResultList = queryResult;
    unitXmlDataMap[modeType][unitType].queryRecycleMap = queryRecycleMap;
    unitXmlDataMap[modeType][unitType].parameterSetList = newCommonSettingMap.values();
    unitXmlDataMap[modeType][unitType].parameterStatusList = newCommonStatusMap.values();
    unitXmlDataMap[modeType][unitType].multiStatusMap = multiStatusMap;
    unitXmlDataMap[modeType][unitType].multiSettingMap = multiSettingMap;

    //分组和布局
    unitXmlDataMap[modeType][unitType].statusGroupMap = statusGroupMap;
    unitXmlDataMap[modeType][unitType].multiStatusGroupMap = multiStatusGroupMap;
    unitXmlDataMap[modeType][unitType].settingGroupMap = settingGroupMap;
    unitXmlDataMap[modeType][unitType].multiSettingGroupMap = multiSettingGroupMap;
    unitXmlDataMap[modeType][unitType].unitLayoutMap = unitLayoutMap;

    unitXmlDataMap[modeType][unitType].targetData = targetMap;
    unitXmlDataMap[modeType][unitType].modeId = modeType;
    if (!kpValid.isEmpty() && kpValid != nullptr)
    {
        unitXmlDataMap[modeType][unitType].kpValid = kpValid;
    }
    if (!isAutoMap.isEmpty())
    {
        unitXmlDataMap[modeType][unitType].isAutoMap = isAutoMap.toInt();
    }
    if (!isShow.isEmpty() || isShow != nullptr)
    {
        unitXmlDataMap[modeType][unitType].isShow = isShow.toInt();
    }
    if (!isModifySetBtn.isEmpty() || isModifySetBtn != nullptr)
    {
        unitXmlDataMap[modeType][unitType].isModifySetBtn = isModifySetBtn.toInt();
    }
    file.close();

    return true;
}

void ProtocolXMLUnitReader::parseNewUnitGroupProtocolXML(
    QDomElement* docElem, QMap<QString, UnitLayout>& unitLayoutMap, QMap<int, UnitMultiTarget>& targetMap,
    QMap<int, ParameterAttribute>& newCommonSettingMap, QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiSettingMap,
    QMap<int, ParameterAttribute>& newCommonStatusMap, QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiStatusMap,
    QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                   //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,   //多目标状态分组
    QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                  //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap,  //多目标设置参数分组
    QStringList& list)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "set")
            {
                node = node.nextSibling();
                continue;
            }
            else if (tagName == "field")
            {
                parseNewQueryResultGroupBaseAttribute(element, targetMap, newCommonSettingMap, newCommonMultiSettingMap, newCommonStatusMap,
                                                      newCommonMultiStatusMap, statusGroupMap, multiStatusGroupMap, settingGroupMap,
                                                      multiSettingGroupMap);
            }
            else if (tagName == "layout")
            {
                QDomNode layoutNode = element.firstChild();
                while (!layoutNode.isNull())
                {
                    QDomElement layoutElement = layoutNode.toElement();
                    if (!layoutElement.isNull())
                    {
                        QString tagName = layoutElement.tagName();
                        if (tagName == "item")
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
                            }
                        }
                    }
                    layoutNode = layoutNode.nextSibling();
                }
            }
            else if (tagName == "order")
            {
                if (!element.attribute("info").isEmpty())
                    list.append(element.attribute("info").split(","));
            }
            parseNewUnitGroupProtocolXML(&element, unitLayoutMap, targetMap, newCommonSettingMap, newCommonMultiSettingMap, newCommonStatusMap,
                                         newCommonMultiStatusMap, statusGroupMap, multiStatusGroupMap, settingGroupMap, multiSettingGroupMap, list);
        }
        node = node.nextSibling();
    }
}

void ProtocolXMLUnitReader::parseNewQueryResultGroupBaseAttribute(
    const QDomElement& element, QMap<int, UnitMultiTarget>& targetMap,  //下面的是按pid顺序存放的参数，上面的是按分组存放的参数
    QMap<int, ParameterAttribute>& newCommonSettingMap, QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiSettingMap,
    QMap<int, ParameterAttribute>& newCommonStatusMap, QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiStatusMap,
    QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                  //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,  //多目标状态分组
    QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                 //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap  //多目标设置参数分组
)
{
    ParameterAttribute base;
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
    if (!isShow.isEmpty() || isShow != NULL)
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

    //处理多目标循环的情况，这里的处理是不区分多目标还是pt的参数，如果有多目标在代码里面处理添加recycle
    //通过判断key是否包含recycle，如果不包含就是pt参数，如果包含recycle就代表是多目标参数
    UnitMultiTarget targetData;
    auto paranNodeName = element.parentNode().nodeName();
    targetData.id = element.parentNode().toElement().attribute("id").toInt();
    targetData.recycleNum = element.parentNode().toElement().attribute("recycleNum");
    targetData.desc = element.parentNode().toElement().attribute("desc");

    QString recycleisShow = element.parentNode().toElement().attribute("isShow").trimmed();  //这个recycle里的内容是否显示
    if (!recycleisShow.isEmpty() || recycleisShow != NULL)
    {
        targetData.isShow = isShow.toInt();
    }

    if (paranNodeName == "recycle")
    {
        if (base.pid >= 100)
        {
            newCommonMultiSettingMap[targetData.id][base.pid] = base;
            targetMap[targetData.id] = targetData;
            multiSettingGroupMap[targetData.id][groupId].append(base);
        }
        else
        {
            targetMap[targetData.id] = targetData;
            newCommonMultiStatusMap[targetData.id][base.pid] = base;
            multiStatusGroupMap[targetData.id][groupId].append(base);
        }
    }
    else
    {
        if (base.pid >= 100)
        {
            newCommonSettingMap[base.pid] = base;
            settingGroupMap[groupId].append(base);
        }
        else
        {
            newCommonStatusMap[base.pid] = base;
            statusGroupMap[groupId].append(base);
        }
    }
}
