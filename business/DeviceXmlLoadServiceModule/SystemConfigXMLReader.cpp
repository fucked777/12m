#include "SystemConfigXMLReader.h"

#include <QDebug>
#include <QFile>

SystemConfigXMLReader::SystemConfigXMLReader(QObject* parent)
    : QObject(parent)
{
}

void SystemConfigXMLReader::loadSystemConfigXML(QString filePath, QMap<int, System>& sysCfgMap, const QMap<QString, UnitXmlDataMap>& unitXmlMap,
                                                const QMap<QString, CmdXMLData>& cmdXMlMap)
{
    QFile file(filePath);

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

    int temp_sysId = -1;
    int temp_devId = -1;
    int temp_extenId = -1;
    int temp_modeType = -1;
    /// parse xml
    parseSystemConfigXML(&root, sysCfgMap, temp_sysId, temp_devId, temp_extenId, temp_modeType, unitXmlMap, cmdXMlMap);

    file.close();
}

void SystemConfigXMLReader::parseSystemConfigXML(QDomElement* docElem, QMap<int, System>& systemMap, int& sysId, int& devId, int& extenId,
                                                 int& modeType, const QMap<QString, UnitXmlDataMap>& unitXmlMap,
                                                 const QMap<QString, CmdXMLData>& cmdXMlMap)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "System")
            {
                System system;
                system.ID = element.attribute("ID").trimmed().toInt(0, 16);
                system.name = element.attribute("name").trimmed();
                system.desc = element.attribute("desc").trimmed();

                systemMap.insert(system.ID, system);
                sysId = system.ID;
            }
            else if (tagName == "Device")
            {
                Device device;
                device.ID = element.attribute("ID").trimmed().toInt(0, 16);
                device.name = element.attribute("name").trimmed();
                device.desc = element.attribute("desc").trimmed();

                systemMap[sysId].deviceMap.insert(device.ID, device);
                devId = device.ID;
            }
            else if (tagName == "Extension")
            {
                Extension extension;
                extension.ID = element.attribute("ID").trimmed().toInt(0, 16);
                extension.extenType = element.attribute("extenType").trimmed();
                extension.desc = element.attribute("desc").trimmed();
                extension.modeAddr = element.attribute("ModeAddress").trimmed().toInt();
                extension.modeLength = element.attribute("ModeLength").trimmed().toInt();
                extension.typeID = element.attribute("typeID", "0x00").trimmed().toInt(0, 16);
                bool haveMode = element.attribute("HaveMode").trimmed().toInt();
                extension.haveMode = haveMode;
                QString isShow = element.attribute("isShow").trimmed();
                if (!isShow.isEmpty() || isShow != nullptr)
                {
                    extension.isShow = isShow.toInt();
                }
                extension.relationDevice = element.attribute("relationDevice").trimmed();

                if (!haveMode)
                {
                    modeType = gNoModeType;
                }

                systemMap[sysId].deviceMap[devId].extensionMap.insert(extension.ID, extension);

                extenId = extension.ID;
            }
            else if (tagName == "Mode")
            {
                ModeCtrl modeCtrl;
                modeCtrl.ID = element.attribute("ID").trimmed().toInt(0, 16);
                modeCtrl.desc = element.attribute("desc").trimmed();

                modeType = modeCtrl.ID;

                /**
                    通过模式类型和分机类型关联控制命令和控制命令上报列表
                **/
                QString sysName = systemMap[sysId].name;
                QString extenType = systemMap[sysId].deviceMap[devId].extensionMap[extenId].extenType;
                if (cmdXMlMap.value(sysName).cmdAttrMap.value(extenType).contains(modeType))
                {
                    modeCtrl.cmdMap = cmdXMlMap.value(sysName).cmdAttrMap.value(extenType).value(modeType);
                }
                /**
                    通过模式类型和分机类型关联该枚举列表
                **/
                if (cmdXMlMap.value(sysName).cmdEnumMap.value(extenType).contains(modeType))
                {
                    modeCtrl.cmdEnumMap = cmdXMlMap.value(sysName).cmdEnumMap.value(extenType).value(modeType);
                }

                systemMap[sysId].deviceMap[devId].extensionMap[extenId].modeCtrlMap.insert(modeType, modeCtrl);
                systemMap[sysId].deviceMap[devId].extensionMap[extenId].currentModeID =
                    systemMap[sysId].deviceMap[devId].extensionMap[extenId].modeCtrlMap.first().ID;  //添加默认模式 cyx V23
            }
            else if (tagName == "Unit")
            {
                if ((modeType == gNoModeType) && !systemMap[sysId].deviceMap[devId].extensionMap[extenId].modeCtrlMap.contains(modeType))
                {
                    ModeCtrl modeCtrl;
                    modeCtrl.ID = gNoModeType;
                    modeCtrl.desc = element.attribute("desc").trimmed();

                    modeType = modeCtrl.ID;

                    /**
                        通过模式类型和分机类型关联控制命令和控制命令上报列表
                    **/
                    QString sysName = systemMap[sysId].name;
                    QString extenType = systemMap[sysId].deviceMap[devId].extensionMap[extenId].extenType;
                    if (cmdXMlMap.value(sysName).cmdAttrMap.value(extenType).contains(modeType))
                    {
                        modeCtrl.cmdMap = cmdXMlMap.value(sysName).cmdAttrMap.value(extenType).value(modeType);
                    }
                    /**
                        通过模式类型和分机类型关联该枚举列表
                    **/
                    if (cmdXMlMap.value(sysName).cmdEnumMap.value(extenType).contains(modeType))
                    {
                        modeCtrl.cmdEnumMap = cmdXMlMap.value(sysName).cmdEnumMap.value(extenType).value(modeType);
                    }

                    systemMap[sysId].deviceMap[devId].extensionMap[extenId].modeCtrlMap.insert(modeType, modeCtrl);
                    systemMap[sysId].deviceMap[devId].extensionMap[extenId].currentModeID = modeCtrl.ID;  //添加默认模式 cyx V23
                }

                Unit unit;
                unit.unitCode = element.attribute("UnitCode").trimmed().toInt();
                unit.unitType = element.attribute("UnitType").trimmed();
                unit.desc = element.attribute("name").trimmed();

                /**
                  通过模式类型和单元类型关联状态上报结果列表
                **/
                QString sysName = systemMap[sysId].name;

                if (unitXmlMap.value(sysName).value(modeType).contains(unit.unitType))
                {
                    unit.modeId = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).modeId;
                    //无分组
                    unit.queryResultList = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).queryResultList;
                    unit.queryRecycleMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).queryRecycleMap;
                    unit.parameterSetList = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).parameterSetList;
                    unit.parameterStatusList = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).parameterStatusList;
                    unit.multiStatusMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).multiStatusMap;
                    unit.multiSettingMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).multiSettingMap;
                    //有分组
                    unit.statusGroupMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).statusGroupMap;
                    unit.multiStatusGroupMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).multiStatusGroupMap;
                    unit.settingGroupMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).settingGroupMap;
                    unit.multiSettingGroupMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).multiSettingGroupMap;
                    unit.unitLayoutMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).unitLayoutMap;
                    //枚举和多目标类型
                    unit.enumMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).enumMap;
                    unit.targetData = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).targetData;

                    unit.kpValid = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).kpValid;
                    unit.isAutoMap = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).isAutoMap;
                    unit.isShow = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).isShow;
                    unit.isModifySetBtn = unitXmlMap.value(sysName).value(modeType).value(unit.unitType).isModifySetBtn;
                }

                systemMap[sysId].deviceMap[devId].extensionMap[extenId].modeCtrlMap[modeType].unitMap.insert(unit.unitCode, unit);
            }
            parseSystemConfigXML(&element, systemMap, sysId, devId, extenId, modeType, unitXmlMap, cmdXMlMap);
        }
        node = node.nextSibling();
    }
}
