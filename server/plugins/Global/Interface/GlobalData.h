#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include "SystemWorkMode.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVariantMap>

/* 获取平台配置文件中的扩展配置部分 */
struct AutorunPolicyData;
struct System;
struct Device;
struct Extension;
struct ModeCtrl;
struct Unit;
struct CmdAttribute;
struct DeviceID;

struct ExtensionStatusReportMessage;
struct UnitReportMessage;
struct MessageAddress;

struct CommunicationInfoMap;

struct DMDataItem;
using DMTypeMap = QMap<QString, QList<DMDataItem>>;
using DMTypeList = QList<DMDataItem>;

struct TaskCenterData;
using TaskCenterMap = QMap<QString, TaskCenterData>;
using TaskCenterList = QList<TaskCenterData>;

struct DataTransmissionCenterData;
using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;
using DataTransmissionCenterList = QList<DataTransmissionCenterData>;

struct StationResManagementData;
using StationResManagementMap = QMap<QString, StationResManagementData>;
using StationResManagementList = QList<StationResManagementData>;

struct ResourceRestructuringData;
using ResourceRestructuringDataList = QList<ResourceRestructuringData>;

struct SatelliteManagementData;
using SatelliteManagementDataList = QList<SatelliteManagementData>;
using SatelliteManagementDataMap = QMap<QString, SatelliteManagementData>;

struct EphemerisDataData;
using EphemerisDataMap = QMap<QString, EphemerisDataData>;
using EphemerisDataList = QList<EphemerisDataData>;

struct AutomateTestParameterBindGroup;
using AutomateTestParameterBindMap = QMap<QString, AutomateTestParameterBindGroup>;
struct AutomateTestPlanItem;
using AutomateTestPlanItemList = QList<AutomateTestPlanItem>;

struct ParamMacroData;
using ParamMacroDataList = QList<ParamMacroData>;
using ParamMacroDataMap = QMap<QString, ParamMacroData>;
struct ParamMacroModeData;

struct ConfigMacroData;
using ConfigMacroMap = QMap<QString, ConfigMacroData>;
using ConfigMacroList = QList<ConfigMacroData>;

struct LinkConfig;
enum class LinkType;
using LinkConfigMap = QMap<LinkType, LinkConfig>;

struct User;
using UserMap = QMap<QString, User>;
using UserList = QList<User>;

struct TimeData;
enum class TimeType;
struct TimeConfigInfo;

struct HeartbeatMessage;
struct ManualMessage;
class GlobalData
{
public:
    /* 获取和设置设备通讯的数据结构 */
    static void setCommunicationInfo(const CommunicationInfoMap& infoMap);
    static CommunicationInfoMap communicationInfo();

    /* 设置和获取设备管理的数据 */
    static void setDeviceManagerInfo(const DMTypeMap& infoMap);
    static bool getDeviceManagerInfo(DMTypeMap& infoMap);
    static bool getDeviceManagerInfo(const QString& type, DMTypeList& infoList);

    /* 设置和获取任务中心数据 */
    static void setTaskCenterInfo(const TaskCenterMap& infoMap);
    static bool getTaskCenterInfo(TaskCenterMap& infoMap);

    /* 设置和获取数传中心数据 */
    static void setDataTransmissionCenterInfo(const DataTransmissionCenterMap& infoMap);
    static bool getDataTransmissionCenterInfo(DataTransmissionCenterMap& infoMap);

    /* 设置和获取站资源数据 */
    static void setStationResManagementInfo(const StationResManagementMap& infoMap);
    static bool getStationResManagementInfo(StationResManagementMap& infoMap);

    /* 设置和获取基带配置数据 */
    static void setResourceRestructuringData(const ResourceRestructuringDataList& dataList);
    static bool getResourceRestructuringData(ResourceRestructuringDataList& dataList);
    static bool getResourceRestructuringData(SystemWorkMode workMode, ResourceRestructuringData& data, MasterSlave masterSlave = MasterSlave::Master);

    /* 设置和获取卫星数据 */
    static void setSatelliteManagementData(const SatelliteManagementDataList& dataList);
    static bool getSatelliteManagementData(SatelliteManagementDataList& dataList);
    static bool getSatelliteManagementData(SatelliteManagementDataMap& dataMap);
    static bool getSatelliteManagementData(const QString& taskCode, SatelliteManagementData& data);

    /* 设置和获取星历数据 */
    static void setEphemerisDataInfo(const EphemerisDataMap& infoMap);
    static bool getEphemerisDataInfo(EphemerisDataMap& infoMap);

    /* 设置和获取自动运行策略 */
    static void setAutorunPolicyData(const AutorunPolicyData& dataMap);
    static bool getAutorunPolicyData(AutorunPolicyData& dataMap);
    static QVariant getAutorunPolicyData(const QString& key, const QVariant& defaultValue = QVariant());

    /* 设置和获取自动化测试数据 */
    static void setAutomateTestParameterBindMap(const AutomateTestParameterBindMap& dataMap);
    static bool getAutomateTestParameterBindMap(AutomateTestParameterBindMap& dataMap);
    static void setAutomateTestPlanItemList(const AutomateTestPlanItemList& dataList);
    static bool getAutomateTestPlanItemList(AutomateTestPlanItemList& dataList);

    /* 设置和获取参数宏数据 */
    static void setParamMacroData(const ParamMacroDataList& dataList);
    /* 极为耗时 20211219改名 */
    static bool getAllParamMacroData(ParamMacroDataMap& dataMap);
    static bool getAllParamMacroData(ParamMacroDataList& dataList);
    static void delParamMacroData(const QString& taskCode);
    static bool getParamMacroData(const QString& taskCode, ParamMacroData& data);
    static bool getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode,
                                      ParamMacroModeData& paramMacroModeItem);  // 获取指定任务代号参数宏下指定模式的参数宏数据

    /* 设置和获取配置宏数据 */
    /* 20211021
     * 更改配置宏的获取
     * 单模式的获取为比较严格 如果没有对应的单模式则获取失败 即使是组合模式中包含此种模式也不行
     * 对于多模式则会尽量的匹配 比如给定多模式STTC+KA低速+X低速 如果没有直接找到组合模式
     * 则会在单模式中匹配 然后组合 注意是在单模式中匹配不会将别的组合模式包含的拿过来用的
     */
    static void setConfigMacroData(const ConfigMacroList& dataList);
    static bool getConfigMacroData(ConfigMacroMap& dataMap);
    static bool getConfigMacroData(ConfigMacroList& dataList);
    static bool getConfigMacroData(SystemWorkMode configMacroWorkMode, ConfigMacroData& configMacroData,
                                   MasterSlave masterSlave = MasterSlave::Master);  // 获取只有一个模式的配置宏
    static bool getConfigMacroData(const QSet<SystemWorkMode>& configMacroWorkModeSet, ConfigMacroData& configMacroData,
                                   MasterSlave masterSlave = MasterSlave::Master);  // 获取包含多个模式的配置宏

    /* 设置和获取链路配置数据 */
    static void setLinkConfigData(const LinkConfigMap& dataMap);
    static bool getLinkConfigData(LinkConfigMap& dataMap);

    /* 设置和获取设备配置数据 */
    static void setSystemMap(const QMap<int, System>& systemMap);
    static QMap<int, System> getSystemMap();
    static System getSystem(int sysId);

    static Device getDevice(int sysId, int deviceId);

    static Extension getExtension(int sysId, int deviceId, int extentId);
    static Extension getExtension(int deviceId);
    static Extension getExtension(const DeviceID& deviceID);
    static Extension getExtension(const MessageAddress& msgAddr);

    static bool isExistsExtension(const DeviceID& deviceID);

    static ModeCtrl getMode(int sysId, int deviceId, int extentId, int modeId);
    static ModeCtrl getMode(int deviceId, int modeId);
    static ModeCtrl getMode(const DeviceID deviceID, int modeId);
    static ModeCtrl getMode(const MessageAddress& msgAddr, int modeId);

    static Unit getUnit(int sysId, int deviceId, int extentId, int modeId, int unitId);
    static Unit getUnit(const DeviceID& deviceID, int modeId, int unitId);
    static Unit getUnit(const MessageAddress& msgAddr, int modeId, int unitId);
    static QMap<int, Unit> getUnit(const MessageAddress& msgAddr, int modeId);

    static CmdAttribute getCmd(int sysId, int deviceId, int extentId, int modeId, int cmdId);
    static CmdAttribute getCmd(const DeviceID& deviceID, int modeId, int cmdId);
    static CmdAttribute getCmd(const MessageAddress& msgAddr, int modeId, int cmdId);

    static QString getSystemName(int sysId);

    static QString getDeviceName(int sysId, int deviceId);

    static QString getExtensionName(int sysId, int deviceId, int extentId);
    static QString getExtensionName(const DeviceID& deviceID);
    static QString getExtensionName(const MessageAddress& msgAddr);

    static QString getModeName(int sysId, int deviceId, int extentId, int modeId);
    static QString getModeName(const DeviceID& deviceID, int modeId);
    static QString getModeName(const MessageAddress& msgAddr, int modeId);

    static QString getUnitName(int sysId, int deviceId, int extentId, int modeId, int unitId);
    static QString getUnitName(int sysId, int deviceId, int extentId, int modeId, int unitId, const QString& separator);
    static QString getUnitName(const DeviceID& deviceId, int modeId, int unitId);
    static QString getUnitName(const MessageAddress& msgAddr, int modeId, int unitId);
    static QString getUnitName(const MessageAddress& msgAddr, int modeId, int unitId, const QString& separator);

    static QString getCmdName(int sysId, int deviceId, int extentId, int modeId, int cmdId);
    static QString getCmdName(const DeviceID& deviceId, int modeId, int cmdId);
    static QString getCmdName(const MessageAddress& msgAddr, int modeId, int cmdId);

    // 分机状态上报数据
    static void setExtenStatusReportData(int sysId, int deviceId, int extenId, const ExtensionStatusReportMessage& statusReportMsg);
    static ExtensionStatusReportMessage getExtenStatusReportData(int sysId, int deviceId, int extenId);
    static ExtensionStatusReportMessage getExtenStatusReportData(const DeviceID& deviceID);
    static ExtensionStatusReportMessage getExtenStatusReportData(int intDeviceID);
    static ExtensionStatusReportMessage getExtenStatusReportData(const QString& deviceId);

    static QMap<QString, QVariant> getReportParamData(const DeviceID& deviceID, int unitId, const QList<QString>& paramIdList, int targetNo = 0);
    static QVariant getReportParamData(const DeviceID& deviceID, int unitId, const QString& paramIdList, int targetNo = 0);

    // 获取单元上报数据
    static UnitReportMessage getUnitReportData(int sysId, int deviceId, int extenId, int modeId, int unitId);

    // 设备是否在线
    static void setDeviceOnline(int sysId, int deviceId, int extenId);
    static bool getDeviceOnline(int sysId, int deviceId, int extenId);
    static bool getDeviceOnline(int deviceId);
    static bool getDeviceOnline(const DeviceID& deviceID);

    // 设备是否是本控
    // 默认本分控是在单元1里面的 S频段射频开关网络中三个设备是三个不同单元
    static bool getDeviceSelfControl(int sysId, int deviceId, int extenId, int unitId = 1);
    static bool getDeviceSelfControl(int deviceId, int unitId = 1);
    static bool getDeviceSelfControl(const DeviceID& deviceID, int unitId = 1);

    // 获取在线设备模式
    static void setOnlineDeviceModeId(const DeviceID& deviceID, int modeId);
    static void setOnlineDeviceModeId(int sysId, int deviceId, int extenId, int modeId);
    static int getOnlineDeviceModeId(int sysId, int deviceId, int extenId);
    static int getOnlineDeviceModeId(const DeviceID& deviceID);

    /* 设置和获取当前用户 */
    static void setCurrentUser(const User& currentuser);
    static User getCurrentUser();

    // 设置和获取是否是自动运行任务
    static void setAutoRunTaskFlag(bool isAutoRunTask);
    static bool getAutoRunTaskFlag();

    // 设置和获取时间配置文件信息
    static void setTimeConfigInfo(const TimeConfigInfo& data);
    static TimeConfigInfo getTimeConfigInfo();

    // 设置和获取使用的时间类型
    static void setUsedTimeType(TimeType data);
    static TimeType getUsedTimeType();

    static void setUsedTimeTypeRedis(TimeType data);
    static TimeType getUsedTimeTypeRedis();

    // 设置和获取时间
    static void setTimeData(const TimeData& data);
    static TimeData getTimeData();

    static QDateTime currentDateTime();
    static QDate currentDate();
    static QTime currentTime();

    /* 设置和获取 服务器在线状态 */
    static void setServiceOnlineStatus();
    static bool getServiceOnlineStatus();

    /* 当前的任务运行信息 */
    static void setTaskRunningInfo(const ManualMessage& mManualMsg);
    static void cleanTaskRunningInfo();
    static ManualMessage getTaskRunningInfo();
    // 获取命令等待时间 单位毫秒
    static int getWaitCmdTimeMS();
    static void waitCmdTimeMS();
    // 获取命令时间 比如等待时间是500毫秒 那么你想等30秒  那次数就变成了60次
    static int getCmdTimeCount(int waitTimer);

    /* 主备机 */
    static void setMasterSlaveChangeTime();
    static QDateTime getMasterSlaveChangeTime();
    static bool masterSlave();
    static HeartbeatMessage masterSlaveInfo();
    static void setMasterSlaveInfo(const HeartbeatMessage&);
};

#endif  // GLOBALDATA_H
