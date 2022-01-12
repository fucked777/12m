#include "GlobalData.h"

#include "AutomateTestSerialize.h"
#include "AutorunPolicyMessageSerialize.h"
#include "CommunicationSerialize.h"
#include "ConfigMacroMessageSerialize.h"
#include "CustomPacketMessageSerialize.h"
#include "DMMessageSerialize.h"
#include "DataTransmissionCenterSerialize.h"
#include "EphemerisDataSerialize.h"
#include "LinkConfigMessageSerialize.h"
#include "ParamMacroMessageSerialize.h"
#include "PlanRunMessageSerialize.h"
#include "PlatformInterface.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ResourceRestructuringMessageSerialize.h"
#include "SatelliteManagementSerialize.h"
#include "StationResManagementSerialize.h"
#include "TaskCenterSerialize.h"
#include "TaskPlanMessageSerialize.h"
#include "TimeMessageSerialize.h"
#include "UserManagerMessageSerialize.h"
#include "MessagePublish.h"
#include "HeartbeatMessage.h"

#include <QCoreApplication>
#include <QVariant>

QString extenStatusKeyName = QString("%1_%2");
QString devOnlineKeyName = QString("OnlineStatus_%1:%2");

void GlobalData::setCommunicationInfo(const CommunicationInfoMap& infoMap)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<CommunicationInfoMap>("CommunicationInfoMap", infoMap);
}
CommunicationInfoMap GlobalData::communicationInfo()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<CommunicationInfoMap>("CommunicationInfoMap");
}

void GlobalData::setDeviceManagerInfo(const DMTypeMap& infoMap)
{
    /* 向redis推送 */
    QString content;
    content << infoMap;
    RedisHelper::getInstance().setData("DeviceManagement", content);
}
bool GlobalData::getDeviceManagerInfo(DMTypeMap& infoMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("DeviceManagement", data))
    {
        data >> infoMap;
        return true;
    }

    return false;
}
bool GlobalData::getDeviceManagerInfo(const QString& type, DMTypeList& infoList)
{
    DMTypeMap infoMap;
    if (!getDeviceManagerInfo(infoMap))
    {
        return false;
    }
    auto find = infoMap.find(type);
    if (find == infoMap.end())
    {
        return false;
    }
    infoList = find.value();
    return true;
}

void GlobalData::setTaskCenterInfo(const TaskCenterMap& infoMap)
{
    /* 向redis推送 */
    QString content;
    content << infoMap;
    RedisHelper::getInstance().setData("TaskCenter", content);
}
bool GlobalData::getTaskCenterInfo(TaskCenterMap& infoMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("TaskCenter", data))
    {
        data >> infoMap;
        return true;
    }
    return false;
}

void GlobalData::setDataTransmissionCenterInfo(const DataTransmissionCenterMap& infoMap)
{
    QByteArray data;
    data << infoMap;
    RedisHelper::getInstance().setData("DataTransmissionCenter", data);
}
bool GlobalData::getDataTransmissionCenterInfo(DataTransmissionCenterMap& infoMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("DataTransmissionCenter", data))
    {
        data >> infoMap;
        return true;
    }
    return false;
}

void GlobalData::setStationResManagementInfo(const StationResManagementMap& infoMap)
{
    QString content;
    content << infoMap;
    RedisHelper::getInstance().setData("StationResManagement", content);
}
bool GlobalData::getStationResManagementInfo(StationResManagementMap& infoMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("StationResManagement", data))
    {
        data >> infoMap;
        return true;
    }
    return false;
}

void GlobalData::setResourceRestructuringData(const ResourceRestructuringDataList& dataList)
{
    QByteArray data;
    data << dataList;
    RedisHelper::getInstance().setData("ResourceRestructuring", data);
}
bool GlobalData::getResourceRestructuringData(ResourceRestructuringDataList& dataList)
{
    QString data;
    if (RedisHelper::getInstance().getData("ResourceRestructuring", data))
    {
        data >> dataList;
        return true;
    }

    return false;
}

bool GlobalData::getResourceRestructuringData(SystemWorkMode workMode, ResourceRestructuringData& data, MasterSlave masterSlave)
{
    ResourceRestructuringDataList dataList;
    if (!GlobalData::getResourceRestructuringData(dataList))
    {
        return false;
    }

    for (auto& item : dataList)
    {
        if (item.workMode == workMode && item.isMaster == masterSlave)
        {
            data = item;
            return true;
        }
    }
    return false;
}

void GlobalData::setSatelliteManagementData(const SatelliteManagementDataList& dataList)
{
    QByteArray data;
    data << dataList;
    RedisHelper::getInstance().setData("SatelliteData", data);
}
bool GlobalData::getSatelliteManagementData(SatelliteManagementDataList& dataList)
{
    QString data;
    if (RedisHelper::getInstance().getData("SatelliteData", data))
    {
        data >> dataList;
        return true;
    }
    return false;
}
bool GlobalData::getSatelliteManagementData(SatelliteManagementDataMap& dataMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("SatelliteData", data))
    {
        data >> dataMap;
        return true;
    }
    return false;
}

bool GlobalData::getSatelliteManagementData(const QString& taskCode, SatelliteManagementData& data)
{
    SatelliteManagementDataMap dataMap;
    QString jsondata;
    if (RedisHelper::getInstance().getData("SatelliteData", jsondata))
    {
        jsondata >> dataMap;
        if (dataMap.contains(taskCode))
        {
            data = dataMap[taskCode];
            return true;
        }
        return false;
    }
    return false;
}

void GlobalData::setEphemerisDataInfo(const EphemerisDataMap& infoMap)
{
    /* 向redis推送 */
    QString content;
    content << infoMap;
    RedisHelper::getInstance().setData("EphemerisData", content);
}
bool GlobalData::getEphemerisDataInfo(EphemerisDataMap& infoMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("EphemerisData", data))
    {
        data >> infoMap;
        return true;
    }

    return false;
}

void GlobalData::setAutorunPolicyData(const AutorunPolicyData& dataMap)
{
    QByteArray data;
    data << dataMap;
    RedisHelper::getInstance().setData("AutorunPolicy", data);
}
bool GlobalData::getAutorunPolicyData(AutorunPolicyData& dataMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("AutorunPolicy", data))
    {
        data >> dataMap;
        return true;
    }
    return false;
}
QVariant GlobalData::getAutorunPolicyData(const QString& key, const QVariant& defaultValue)
{
    AutorunPolicyData tempMap;
    if (!getAutorunPolicyData(tempMap))
    {
        return defaultValue;
    }
    return tempMap.policy.value(key, defaultValue);
}

bool GlobalData::getAutomateTestParameterBindMap(AutomateTestParameterBindMap& dataMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("AutomateTestParameterBindMap", data))
    {
        data >> dataMap;
        return true;
    }

    return false;
}
void GlobalData::setAutomateTestParameterBindMap(const AutomateTestParameterBindMap& dataMap)
{
    QByteArray data;
    data << dataMap;
    RedisHelper::getInstance().setData("AutomateTestParameterBindMap", data);
}
bool GlobalData::getAutomateTestPlanItemList(AutomateTestPlanItemList& dataList)
{
    QString data;
    if (RedisHelper::getInstance().getData("AutomateTestPlanItemData", data))
    {
        data >> dataList;
        return true;
    }

    return false;
}
void GlobalData::setAutomateTestPlanItemList(const AutomateTestPlanItemList& dataList)
{
    QByteArray data;
    data << dataList;
    RedisHelper::getInstance().setData("AutomateTestPlanItemData", data);
}

void GlobalData::setParamMacroData(const ParamMacroDataList& dataList)
{
    for(auto&item: dataList)
    {
        QString data;
        data << item;
        RedisHelper::getInstance().hsetData("ParamMacroDataMap", item.taskCode, data);
    }
}

bool GlobalData::getAllParamMacroData(ParamMacroDataMap& dataMap)
{
    QMap<QString, QString> data;
    if (!RedisHelper::getInstance().hgetData("ParamMacroDataMap", data))
    {
        return false;
    }

    for(auto iter = data.begin(); iter != data.end(); ++iter)
    {
        ParamMacroData paramMacroData;
        iter.value() >> paramMacroData;
        dataMap.insert(iter.key(), paramMacroData);
    }
    return true;
}

bool GlobalData::getAllParamMacroData(ParamMacroDataList& dataList)
{
    QMap<QString, QString> data;
    if (!RedisHelper::getInstance().hgetData("ParamMacroDataMap", data))
    {
        return false;
    }

    for(auto iter = data.begin(); iter != data.end(); ++iter)
    {
        ParamMacroData paramMacroData;
        iter.value() >> paramMacroData;
        dataList << paramMacroData;
    }
    return true;
}
bool GlobalData::getParamMacroData(const QString& taskCode, ParamMacroData& paramMacroData)
{
    QString data;
    if (RedisHelper::getInstance().hgetData("ParamMacroDataMap", taskCode, data))
    {
        data >> paramMacroData;
        return true;
    }
    return false;
}

bool GlobalData::getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode, ParamMacroModeData& paramMacroModeItem)
{
    ParamMacroData paramMacroData;
    if (!getParamMacroData(taskCode, paramMacroData))
    {
        return false;
    }
    if (!paramMacroData.modeParamMap.contains(workMode))
    {
        return false;
    }
    paramMacroModeItem = paramMacroData.modeParamMap.value(workMode);

    return true;
}

void GlobalData::delParamMacroData(const QString& taskCode)
{
    RedisHelper::getInstance().hdelKey("ParamMacroDataMap", taskCode);
}

void GlobalData::setConfigMacroData(const ConfigMacroList& dataList)
{
    QByteArray data;
    data << dataList;
    RedisHelper::getInstance().setData("ConfigMacro", data);
}

bool GlobalData::getConfigMacroData(ConfigMacroMap& dataMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("ConfigMacro", data))
    {
        data >> dataMap;
        return true;
    }

    return false;
}

bool GlobalData::getConfigMacroData(ConfigMacroList& dataList)
{
    QString data;
    if (RedisHelper::getInstance().getData("ConfigMacro", data))
    {
        data >> dataList;
        return true;
    }

    return false;
}
bool GlobalData::getConfigMacroData(SystemWorkMode configMacroWorkMode, ConfigMacroData& configMacroData, MasterSlave masterSlave)
{
    QString data;
    if (!RedisHelper::getInstance().getData("ConfigMacro", data))
    {
        return false;
    }

    ConfigMacroList dataList;
    data >> dataList;

    auto desName = ConfigMacroMessageHelper::createSearchName(configMacroWorkMode, masterSlave);
    for (auto& item : dataList)
    {
        if (desName == item.configMacroName)
        {
            configMacroData = item;
            return true;
        }
    }
    return false;
}

bool GlobalData::getConfigMacroData(const QSet<SystemWorkMode>& configMacroWorkModeSet, ConfigMacroData& configMacroData, MasterSlave masterSlave)
{
    QString data;
    if (!RedisHelper::getInstance().getData("ConfigMacro", data))
    {
        return false;
    }

    ConfigMacroList dataList;
    data >> dataList;

    auto desName = ConfigMacroMessageHelper::createSearchName(configMacroWorkModeSet, masterSlave);
    /* 首先找对应的组合模式 */
    for (auto& item : dataList)
    {
        if (desName == item.configMacroName)
        {
            configMacroData = item;
            return true;
        }
    }

    /* 没找到到组合的 找单模式拼接 */
    ConfigMacroDataMap signalModeMap;
    for (auto& item : dataList)
    {
        if (item.configMacroCmdModeMap.size() == 1)
        {
            signalModeMap.insert(item.configMacroName, item);
        }
    }

    configMacroData.configMacroName = desName;
    configMacroData.configMacroID = desName;
    configMacroData.desc = desName;
    configMacroData.configMacroCmdModeMap.clear();
    for (auto& item : configMacroWorkModeSet)
    {
        auto signalModeName = ConfigMacroMessageHelper::createSearchName(item, masterSlave);
        auto tempConfigMacroData = signalModeMap.value(signalModeName);
        /* 没找到单模式 */
        if (tempConfigMacroData.configMacroCmdModeMap.isEmpty())
        {
            return false;
        }
        configMacroData.configMacroCmdModeMap.insert(item, tempConfigMacroData.configMacroCmdModeMap.first());
    }
    return true;
}

void GlobalData::setLinkConfigData(const LinkConfigMap& dataMap)
{
    QByteArray data;
    data << dataMap;
    RedisHelper::getInstance().setData("LinkConfig", data);
}
bool GlobalData::getLinkConfigData(LinkConfigMap& dataMap)
{
    QString data;
    if (RedisHelper::getInstance().getData("LinkConfig", data))
    {
        data >> dataMap;
        return true;
    }

    return false;
}

void GlobalData::setSystemMap(const QMap<int, System>& systemMap)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<QMap<int, System>>("SystemMap", systemMap);
}

QMap<int, System> GlobalData::getSystemMap()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<QMap<int, System>>("SystemMap");
}

System GlobalData::getSystem(int sysId)
{
    auto var = getSystemMap();
    return var[sysId];
}

Device GlobalData::getDevice(int sysId, int deviceId)
{
    auto var = getSystemMap();
    return var[sysId].deviceMap[deviceId];
}

Unit GlobalData::getUnit(int sysId, int deviceId, int extentId, int modeId, int unitId)
{
    auto var = getSystemMap();
    return var[sysId].deviceMap[deviceId].extensionMap[extentId].modeCtrlMap[modeId].unitMap[unitId];
}

Unit GlobalData::getUnit(const DeviceID& deviceID, int modeId, int unitId)
{
    return getUnit(deviceID.sysID, deviceID.devID, deviceID.extenID, modeId, unitId);
}

Unit GlobalData::getUnit(const MessageAddress& msgAddr, int modeId, int unitId)
{
    return getUnit(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId, unitId);
}

QMap<int, Unit> GlobalData::getUnit(const MessageAddress& msgAddr, int modeId) { return getMode(msgAddr, modeId).unitMap; }

CmdAttribute GlobalData::getCmd(int sysId, int deviceId, int extentId, int modeId, int cmdId)
{
    auto var = getSystemMap();
    return var[sysId].deviceMap[deviceId].extensionMap[extentId].modeCtrlMap[modeId].cmdMap[cmdId];
}

CmdAttribute GlobalData::getCmd(const DeviceID& deviceID, int modeId, int cmdId)
{
    return getCmd(deviceID.sysID, deviceID.devID, deviceID.extenID, modeId, cmdId);
}
CmdAttribute GlobalData::getCmd(const MessageAddress& msgAddr, int modeId, int cmdId)
{
    return getCmd(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId, cmdId);
}
Extension GlobalData::getExtension(int sysId, int deviceId, int extentId)
{
    auto var = getSystemMap();
    return var[sysId].deviceMap[deviceId].extensionMap[extentId];
}

Extension GlobalData::getExtension(int deviceId)
{
    DeviceID deviceID(deviceId);
    return getExtension(deviceID.sysID, deviceID.devID, deviceID.extenID);
}

Extension GlobalData::getExtension(const DeviceID& deviceID) { return getExtension(deviceID.sysID, deviceID.devID, deviceID.extenID); }

Extension GlobalData::getExtension(const MessageAddress& msgAddr) { return getExtension(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb); }

bool GlobalData::isExistsExtension(const DeviceID& deviceID) { return getExtension(deviceID).ID == -1 ? false : true; }

ModeCtrl GlobalData::getMode(int sysId, int deviceId, int extentId, int modeId)
{
    auto var = getSystemMap();
    return var[sysId].deviceMap[deviceId].extensionMap[extentId].modeCtrlMap[modeId];
}

ModeCtrl GlobalData::getMode(int deviceId, int modeId)
{
    DeviceID deviceID(deviceId);
    return getMode(deviceID.sysID, deviceID.devID, deviceID.extenID, modeId);
}

ModeCtrl GlobalData::getMode(const DeviceID deviceID, int modeId) { return getMode(deviceID.sysID, deviceID.devID, deviceID.extenID, modeId); }

ModeCtrl GlobalData::getMode(const MessageAddress& msgAddr, int modeId)
{
    return getMode(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId);
}

QString GlobalData::getSystemName(int sysId) { return getSystem(sysId).desc; }

QString GlobalData::getDeviceName(int sysId, int deviceId) { return getDevice(sysId, deviceId).desc; }

QString GlobalData::getExtensionName(int sysId, int deviceId, int extentId) { return getExtension(sysId, deviceId, extentId).desc; }

QString GlobalData::getExtensionName(const MessageAddress& msgAddr)
{
    return getExtensionName(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb);
}

QString GlobalData::getExtensionName(const DeviceID& deviceID) { return getExtensionName(deviceID.sysID, deviceID.devID, deviceID.extenID); }

QString GlobalData::getModeName(int sysId, int deviceId, int extentId, int modeId) { return getMode(sysId, deviceId, extentId, modeId).desc; }

QString GlobalData::getModeName(const DeviceID& deviceID, int modeId)
{
    return getModeName(deviceID.sysID, deviceID.devID, deviceID.extenID, modeId);
}

QString GlobalData::getModeName(const MessageAddress& msgAddr, int modeId)
{
    return getModeName(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId);
}

QString GlobalData::getUnitName(int sysId, int deviceId, int extentId, int modeId, int unitId)
{
    auto unit = getUnit(sysId, deviceId, extentId, modeId, unitId);
    return unit.isValid() ? unit.desc : QString();
}

QString GlobalData::getUnitName(int sysId, int deviceId, int extentId, int modeId, int unitId, const QString& separator)
{
    auto unit = getUnit(sysId, deviceId, extentId, modeId, unitId);
    if (!unit.isValid())
    {
        return QString();
    }
    QStringList names;
    names << getSystemName(sysId) << getDeviceName(sysId, deviceId) << getExtensionName(sysId, deviceId, extentId)
          << getModeName(sysId, deviceId, extentId, modeId) << unit.desc;

    return names.join(separator);
}

QString GlobalData::getUnitName(const DeviceID& deviceId, int modeId, int unitId)
{
    return getUnitName(deviceId.sysID, deviceId.devID, deviceId.extenID, modeId, unitId);
}

QString GlobalData::getUnitName(const MessageAddress& msgAddr, int modeId, int unitId)
{
    return getUnitName(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId, unitId);
}

QString GlobalData::getUnitName(const MessageAddress& msgAddr, int modeId, int unitId, const QString& separator)
{
    return getUnitName(msgAddr.systemNumb, msgAddr.deviceNumb, msgAddr.extenNumb, modeId, unitId, separator);
}

QString GlobalData::getCmdName(int sysId, int deviceId, int extentId, int modeId, int cmdId)
{
    return getCmd(sysId, deviceId, extentId, modeId, cmdId).desc;
}

QString GlobalData::getCmdName(const DeviceID& deviceId, int modeId, int cmdId)
{
    return getCmdName(deviceId.sysID, deviceId.devID, deviceId.extenID, modeId, cmdId);
}

QString GlobalData::getCmdName(const MessageAddress& msgAddr, int modeId, int cmdId) { return getCmd(msgAddr, modeId, cmdId).desc; }

void GlobalData::setExtenStatusReportData(int sysId, int deviceId, int extenId, const ExtensionStatusReportMessage& statusReportMsg)
{
    auto extenName = getExtensionName(sysId, deviceId, extenId);
    if (extenName.isEmpty())
    {
        extenName = "未知分机";
    }

    DeviceID deviceID(sysId, deviceId, extenId);
    int intDeviceID;
    deviceID >> intDeviceID;

    QString key = extenStatusKeyName.arg(QString::number(intDeviceID, 16).toUpper(), extenName);

    QString data;
    data << statusReportMsg;

    // 3s后设备状态数据消失
    RedisHelper::getInstance().setData(key, data, 6);
}
ExtensionStatusReportMessage GlobalData::getExtenStatusReportData(int sysId, int deviceId, int extenId)
{
    auto extenName = getExtensionName(sysId, deviceId, extenId);
    if (extenName.isEmpty())
    {
        extenName.append("未知分机:");
    }

    DeviceID deviceID(sysId, deviceId, extenId);
    int intDeviceID;
    deviceID >> intDeviceID;

    QString key = extenStatusKeyName.arg(QString::number(intDeviceID, 16).toUpper(), extenName);

    QString data;
    RedisHelper::getInstance().getData(key, data);

    ExtensionStatusReportMessage msg;
    JsonReader reader(data);
    reader& msg;

    return msg;
}

ExtensionStatusReportMessage GlobalData::getExtenStatusReportData(const DeviceID& deviceID)
{
    return getExtenStatusReportData(deviceID.sysID, deviceID.devID, deviceID.extenID);
}

ExtensionStatusReportMessage GlobalData::getExtenStatusReportData(int intDeviceID)
{
    DeviceID devID;
    devID << intDeviceID;
    return getExtenStatusReportData(devID);
}

ExtensionStatusReportMessage GlobalData::getExtenStatusReportData(const QString& deviceId)
{
    return getExtenStatusReportData(deviceId.toInt(nullptr, 16));
}
QMap<QString, QVariant> GlobalData::getReportParamData(const DeviceID& deviceID, int unitId, const QList<QString>& paramIdList, int targetNo)
{
    QMap<QString, QVariant> resultParamMap;

    auto extenStatusReportMsg = getExtenStatusReportData(deviceID);
    auto unitReportParam = extenStatusReportMsg.unitReportMsgMap.value(unitId);

    for (auto& paramId : paramIdList)
    {
        if (unitReportParam.paramMap.contains(paramId))  // 公共参数
        {
            resultParamMap[paramId] = unitReportParam.paramMap[paramId];
        }

        for (auto targetNo : unitReportParam.multiTargetParamMap.keys())  // 这里循环多目标是多目标参数的id不同的情况多目标，比如一体化+XXX
        {
            auto targetParamMap = unitReportParam.multiTargetParamMap.value(targetNo);
            if (targetParamMap.contains(paramId))
            {
                resultParamMap[paramId] = targetParamMap[paramId];
            }
        }
    }

    if (targetNo != 0 && unitReportParam.multiTargetParamMap.contains(targetNo))  // 这里循环多目标是多目标参数的id相同的多目标，比如扩频
    {
        auto multiTargetParamMap = unitReportParam.multiTargetParamMap[targetNo];
        for (auto& paramId : paramIdList)
        {
            if (multiTargetParamMap.contains(paramId))
            {
                resultParamMap[paramId] = multiTargetParamMap[paramId];
            }
        }
    }

    return resultParamMap;
}
QVariant GlobalData::getReportParamData(const DeviceID& deviceID, int unitId, const QString& paramId, int targetNo)
{
    auto extenStatusReportMsg = getExtenStatusReportData(deviceID);
    auto unitReportParam = extenStatusReportMsg.unitReportMsgMap.value(unitId);

    if (unitReportParam.paramMap.contains(paramId))  // 公共参数
    {
        return unitReportParam.paramMap[paramId];
    }

    for (auto targetNo : unitReportParam.multiTargetParamMap.keys())  // 这里循环多目标是多目标参数的id不同的情况多目标，比如一体化+XXX
    {
        auto targetParamMap = unitReportParam.multiTargetParamMap.value(targetNo);
        if (targetParamMap.contains(paramId))
        {
            return targetParamMap[paramId];
        }
    }

    if (targetNo != 0 && unitReportParam.multiTargetParamMap.contains(targetNo))  // 这里循环多目标是多目标参数的id相同的多目标，比如扩频
    {
        auto multiTargetParamMap = unitReportParam.multiTargetParamMap[targetNo];

        if (multiTargetParamMap.contains(paramId))
        {
            return multiTargetParamMap[paramId];
        }
    }

    return QVariant();
}
UnitReportMessage GlobalData::getUnitReportData(int sysId, int deviceId, int extenId, int modeId, int unitId)
{
    auto extenStatusReportMsg = getExtenStatusReportData(sysId, deviceId, extenId);
    if (modeId != extenStatusReportMsg.modeId)
    {
        return UnitReportMessage();
    }

    return extenStatusReportMsg.unitReportMsgMap[unitId];
}

void GlobalData::setDeviceOnline(int sysId, int deviceId, int extenId)
{
    auto extenName = getExtensionName(sysId, deviceId, extenId);
    if (extenName.isEmpty())
    {
        extenName = "未知分机";
    }
    // QStringList list;
    // list << QString::number(sysId) << QString::number(deviceId) << QString::number(extenId);
    // QString key = devOnlineKeyName.arg(extenName, list.join("_"));

    QString key = devOnlineKeyName.arg(extenName, DeviceID(sysId, deviceId, extenId).toHex());
    // 6s后设备状态数据消失
    RedisHelper::getInstance().setData(key, "在线", 6);

    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<QDateTime>(key, GlobalData::currentDateTime());
}
bool GlobalData::getDeviceOnline(int sysId, int deviceId, int extenId)
{
    auto extenName = getExtensionName(sysId, deviceId, extenId);
    if (extenName.isEmpty())
    {
        extenName = "未知分机";
    }

    QString key = devOnlineKeyName.arg(extenName, DeviceID(sysId, deviceId, extenId).toHex());

    auto platformObjectTools = PlatformObjectTools::instance();
    auto offset = GlobalData::currentDateTime().toMSecsSinceEpoch() - platformObjectTools->getObjCp<QDateTime>(key).toMSecsSinceEpoch();
    return offset >= 0 && offset <= 6000;

    // QStringList list;
    // list << QString::number(sysId) << QString::number(deviceId) << QString::number(extenId);
    // QString online;
    // RedisHelper::getInstance().getData(key, online);
    // return online == "在线" ? true : false;
}

bool GlobalData::getDeviceOnline(int deviceId)
{
    DeviceID deviceID(deviceId);
    return getDeviceOnline(deviceID);
}

bool GlobalData::getDeviceOnline(const DeviceID& deviceID) { return getDeviceOnline(deviceID.sysID, deviceID.devID, deviceID.extenID); }

bool GlobalData::getDeviceSelfControl(int sysId, int deviceId, int extenId, int unitId)
{
    auto extenStatusMsg = getExtenStatusReportData(sysId, deviceId, extenId);
    if (extenStatusMsg.modeId == -1)  // 设备可能不在线
    {
        return false;
    }

    if (sysId == 1 || (sysId == 3 && deviceId == 1 && (extenId == 3 || extenId == 4)))
    {
        unitId = 2;  //天线的本分控在第二个单元,Ka高数数传高频箱和X高频箱的本分控在单元2
    }
    else if (sysId == 3 && deviceId == 1 && (extenId == 1 || extenId == 2))
    {
        unitId = 4;  // Ka测控跟踪高频箱和Ka低速数传及跟踪高频箱的本分控在单元4
    }

    if (!extenStatusMsg.unitReportMsgMap.contains(unitId))  // 单元id错误
    {
        return false;
    }

    auto unitParamMsg = extenStatusMsg.unitReportMsgMap[unitId];
    if (!unitParamMsg.paramMap.contains(
            "ControlMode"))  // ControlMode 配置文件中，本分控参数id要为ControlMode， 1:分控 2:本控;  本分控状态需要通过参数id为:ControlMode进行判断
    {
        return false;
    }

    return unitParamMsg.paramMap["ControlMode"] == 1 ? false : true;
}

bool GlobalData::getDeviceSelfControl(int deviceId, int unitId)
{
    DeviceID deviceID(deviceId);
    return getDeviceSelfControl(deviceID.sysID, deviceID.devID, deviceID.extenID, unitId);
}

bool GlobalData::getDeviceSelfControl(const DeviceID& deviceID, int unitId)
{
    return getDeviceSelfControl(deviceID.sysID, deviceID.devID, deviceID.extenID, unitId);
}
void GlobalData::setOnlineDeviceModeId(const DeviceID& deviceID, int modeId)
{
    int intDeviceID;
    deviceID >> intDeviceID;

    QString strDeviceID = QString::number(intDeviceID, 16).toUpper();

    // 3s后设备状态数据消失
    RedisHelper::getInstance().hsetData("在线设备模式Id", strDeviceID, QString::number(modeId));
}

void GlobalData::setOnlineDeviceModeId(int sysId, int devId, int extenId, int modeId)
{
    DeviceID deviceID(sysId, devId, extenId);
    setOnlineDeviceModeId(deviceID, modeId);
}

int GlobalData::getOnlineDeviceModeId(int sysId, int deviceId, int extenId)
{
    DeviceID deviceID(sysId, deviceId, extenId);

    return getOnlineDeviceModeId(deviceID);
}
int GlobalData::getOnlineDeviceModeId(const DeviceID& deviceID)
{
    int intDeviceID;
    deviceID >> intDeviceID;

    QString strDeviceID = QString::number(intDeviceID, 16).toUpper();

    QString modeId = "-1";
    RedisHelper::getInstance().hgetData("在线设备模式Id", strDeviceID, modeId);

    return modeId.toInt();
}

void GlobalData::setCurrentUser(const User& currentuser)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<User>("CurrentUser", currentuser);
}

User GlobalData::getCurrentUser()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<User>("CurrentUser");
}

void GlobalData::setAutoRunTaskFlag(bool isAutoRunTask)
{
    SystemLogPublish::specificTipsMsg(isAutoRunTask?"切换为自动运行":"切换为手动运行");
    RedisHelper::getInstance().setData("AutoRunTaskFlag(是否自动化运行任务)", isAutoRunTask ? "1" : "0");
}

bool GlobalData::getAutoRunTaskFlag()
{
    QString value;
    if (RedisHelper::getInstance().getData("AutoRunTaskFlag(是否自动化运行任务)", value))
    {
        if (value == "1")
        {
            return true;
        }
    }
    return false;
}

void GlobalData::setTimeConfigInfo(const TimeConfigInfo& data)
{
    QByteArray value;
    value << data;
    RedisHelper::getInstance().setData("TimeConfigInfo(时间配置文件信息)", value);
}

TimeConfigInfo GlobalData::getTimeConfigInfo()
{
    QString data;
    RedisHelper::getInstance().getData("TimeConfigInfo(时间配置文件信息)", data);

    TimeConfigInfo info;
    data >> info;
    return info;
}

TimeType GlobalData::getUsedTimeType()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<TimeData>("SystemDateTime").usedTimeType;
}

void GlobalData::setUsedTimeType(TimeType data)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<TimeType>("UsedDateTimeType", data);
}

void GlobalData::setUsedTimeTypeRedis(TimeType data)
{
    RedisHelper::getInstance().setData("UsedDateTimeType", QString::number(int(data)));
}

TimeType GlobalData::getUsedTimeTypeRedis()
{
    QString data;
    RedisHelper::getInstance().getData("UsedDateTimeType", data);
    return TimeType(data.toInt());
}

void GlobalData::setTimeData(const TimeData& data)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<TimeData>("SystemDateTime", data);
}

TimeData GlobalData::getTimeData()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<TimeData>("SystemDateTime");
}

QDateTime GlobalData::currentDateTime()
{
    auto timeData = getTimeData();
    QDateTime curTime;
    switch (timeData.usedTimeType)
    {
    case TimeType::BCodeTime: { curTime = timeData.bCodeTime; break; }
    case TimeType::NTPTime: //{ curTime = timeData.ntpTime; break; }
    case TimeType::Unknown:
    case TimeType::SystemTime: { curTime = QDateTime::currentDateTime(); break; }
    }
    return curTime.isValid() ? curTime : QDateTime::currentDateTime();
}

QDate GlobalData::currentDate() { return currentDateTime().date(); }

QTime GlobalData::currentTime() { return currentDateTime().time(); }

void GlobalData::setServiceOnlineStatus() { RedisHelper::getInstance().setData(PlatformConfigTools::instanceID(), "true", 5); }

bool GlobalData::getServiceOnlineStatus()
{
    QString data;
    if (!RedisHelper::getInstance().getData(PlatformConfigTools::instanceID(), data))
    {
        return false;
    }
    return QVariant(data).toBool();
}
void GlobalData::setTaskRunningInfo(const ManualMessage& mManualMsg)
{
    QString json;
    json << mManualMsg;
    RedisHelper::getInstance().setData("TaskRunningInfo", json);
}
void GlobalData::cleanTaskRunningInfo() { RedisHelper::getInstance().setData("TaskRunningInfo", QString()); }

ManualMessage GlobalData::getTaskRunningInfo()
{
    ManualMessage mManualMsg;
    QString json;
    if (!RedisHelper::getInstance().getData("TaskRunningInfo", json))
    {
        return ManualMessage();
    }
    json >> mManualMsg;
    return mManualMsg;
}
void GlobalData::waitCmdTimeMS() { QThread::msleep(getWaitCmdTimeMS()); }
int GlobalData::getWaitCmdTimeMS() { return 250; }
int GlobalData::getCmdTimeCount(int waitTimer) { return static_cast<int>(1000.0 / getWaitCmdTimeMS() * waitTimer); }

bool GlobalData::masterSlave()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<HeartbeatMessage>("HaStatus").status == MasterSlaveStatus::Master;
}
HeartbeatMessage GlobalData::masterSlaveInfo()
{
    auto platformObjectTools = PlatformObjectTools::instance();
    return platformObjectTools->getObjCp<HeartbeatMessage>("HaStatus");
}
void GlobalData::setMasterSlaveInfo(const HeartbeatMessage& info)
{
    auto platformObjectTools = PlatformObjectTools::instance();
    platformObjectTools->updataObj<HeartbeatMessage>("HaStatus", info);
}
void GlobalData::setMasterSlaveChangeTime()
{
    auto timeStr = currentDateTime().toString();
    RedisHelper::getInstance().setData("MasterSlaveChangeTime", timeStr);
}

QDateTime GlobalData::getMasterSlaveChangeTime()
{
    QString timeStr;
    RedisHelper::getInstance().getData("MasterSlaveChangeTime", timeStr);
    return QDateTime::fromString(timeStr);
}
