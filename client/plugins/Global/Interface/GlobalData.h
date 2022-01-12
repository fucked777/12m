#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include "DevProtocol.h"
#include "SystemWorkMode.h"
#include <QLabel>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

/* 获取平台配置文件中的扩展配置部分 */
struct AutorunPolicyData;
struct System;
struct Device;
struct Extension;
struct ModeCtrl;
struct Unit;
struct CmdAttribute;
struct ControlAlignment;

struct DeviceID;

struct ExtensionStatusReportMessage;

struct MessageAddress;

struct TaskCenterData;
using TaskCenterMap = QMap<QString, TaskCenterData>;
using TaskCenterList = QList<TaskCenterData>;

struct DataTransmissionCenterData;
using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;
using DataTransmissionCenterList = QList<DataTransmissionCenterData>;
struct ResourceRestructuringData;
using ResourceRestructuringDataList = QList<ResourceRestructuringData>;

struct SatelliteManagementData;
using SatelliteManagementDataList = QList<SatelliteManagementData>;
using SatelliteManagementDataMap = QMap<QString, SatelliteManagementData>;

// struct DeviceWorkTask;
// using DeviceWorkTaskList = QList<DeviceWorkTask>;
// struct DataTranWorkTask;
// using DataTranWorkTaskList = QList<DataTranWorkTask>;

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

struct ConfigMacroWorkMode;
using ConfigMacroWorkModeMap = QMap<SystemWorkMode, ConfigMacroWorkMode>;
using ConfigMacroWorkModeList = QList<ConfigMacroWorkMode>;

struct User;
using UserMap = QMap<QString, User>;
using UserList = QList<User>;

struct TimeData;
enum class TimeType;
struct ManualMessage;

struct HeartbeatMessage;

class GlobalData
{
public:
    /* 获取数传中心数据 */
    static bool getDataTransmissionCenterInfo(DataTransmissionCenterMap& infoMap);

    /* 获取任务中心数据 */
    static bool getTaskCenterInfo(TaskCenterMap& infoMap);

    /* 设置和获取自动化测试参数绑定数据 */
    static bool getAutomateTestParameterBindMap(AutomateTestParameterBindMap& dataMap);
    static bool getAutomateTestPlanItemList(AutomateTestPlanItemList& dataList);

    /* 设置和获取基带配置数据 */
    static bool getResourceRestructuringData(ResourceRestructuringDataList& dataList);

    /* 获取卫星数据 */
    static bool getSatelliteManagementData(SatelliteManagementDataList& dataList);
    static bool getSatelliteManagementData(SatelliteManagementDataMap& dataMap);
    static bool getSatelliteManagementData(const QString& taskCode, SatelliteManagementData& data);

    static SatelliteManagementDataList getSatelliteManagementDataByObject();

    /* 设置和获取参数宏数据 */
    static void setParamMacroData(const ParamMacroDataList& dataList);
    static bool getParamMacroData(ParamMacroDataMap& dataMap);
    static bool getParamMacroData(ParamMacroDataList& dataList);
    static bool getParamMacroData(QMap<QString, QString>& dataList);
    static bool getParamMacroData(const QString& taskCode, ParamMacroData& paramMacroData);
    static bool getParamMacroModeData(const QString& taskCode, SystemWorkMode workMode,
                                      ParamMacroModeData& paramMacroModeItem);  // 获取指定任务代号参数宏下指定模式的参数宏数据

    /* 获取配置宏数据 */
    /* 20211021
     * 更改配置宏的获取
     * 单模式的获取为比较严格 如果没有对应的单模式则获取失败 即使是组合模式中包含此种模式也不行
     * 对于多模式则会尽量的匹配 比如给定多模式STTC+KA低速+X低速 如果没有直接找到组合模式
     * 则会在单模式中匹配 然后组合 注意是在单模式中匹配不会将别的组合模式包含的拿过来用的
     */
    static bool getConfigMacroData(ConfigMacroMap& dataMap);
    static bool getConfigMacroData(ConfigMacroList& dataList);
    static bool getConfigMacroData(SystemWorkMode configMacroWorkMode, ConfigMacroData& configMacroData,
                                   MasterSlave masterSlave = MasterSlave::Master);  // 获取只有一个模式的配置宏
    static bool getConfigMacroData(const QSet<SystemWorkMode>& configMacroWorkModeSet, ConfigMacroData& configMacroData,
                                   MasterSlave masterSlave = MasterSlave::Master);  // 获取包含多个模式的配置宏

    /* 设置和获取生成配置宏界面的工作模式配置文件数据 */
    static void setConfigMacroWorkModeData(const ConfigMacroWorkModeList& dataList);
    static bool getConfigMacroWorkModeData(ConfigMacroWorkModeList& dataList);
    static bool getConfigMacroWorkModeData(ConfigMacroWorkModeMap& dataList);

    /* 设置和获取自动运行策略 */
    static void setAutorunPolicyData(const AutorunPolicyData& dataMap);
    static bool getAutorunPolicyData(AutorunPolicyData& dataMap);
    static QVariant getAutorunPolicyData(const QString& key, const QVariant& defaultValue = QVariant());

    /*设置设备控件的配置数据*/
    static void setDeviceControlData(const ControlAlignment& controlAlignData);
    static ControlAlignment getDeviceControlData();

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
    static ExtensionStatusReportMessage getExtenStatusReportData(int sysId, int deviceId, int extenId);
    static ExtensionStatusReportMessage getExtenStatusReportData(const DeviceID& deviceID);
    static ExtensionStatusReportMessage getExtenStatusReportData(int intDeviceID);
    static ExtensionStatusReportMessage getExtenStatusReportData(const QString& deviceId);
    /*
     * 从平台对象获取分机状态上报数据,获取速度比redis更快  其中平台对象里面的数据来源于主界面的一个线程处理里,所以主界面模块最好不要屏蔽
     * 注：原来的getExtenStatusReportData暂不要删掉，因为线程存进对象的数据就是从这个函数取的
     */
    static ExtensionStatusReportMessage getExtenStatusReportDataByObject(int sysId, int deviceId, int extenId);
    static ExtensionStatusReportMessage getExtenStatusReportDataByObject(int intDeviceID);
    static ExtensionStatusReportMessage getExtenStatusReportDataByObject(const QString& deviceId);
    static ExtensionStatusReportMessage getExtenStatusReportDataByObject(DeviceID deviceID);

    static QMap<QString, QVariant> getReportParamData(const DeviceID& deviceID, int unitId, const QList<QString>& paramIdList, int targetNo = 0);
    static QVariant getReportParamData(const DeviceID& deviceID, int unitId, const QString& paramId, int targetNo = 0);

    // 设备是否在线
    static bool getDeviceOnline(int sysId, int deviceId, int extenId);
    static bool getDeviceOnline(int deviceId);
    static bool getDeviceOnline(const DeviceID& deviceID);
    /*
     * 从平台对象获取设备是否在线,获取速度比redis更快  其中平台对象里面的数据来源于主界面的一个线程处理里,所以主界面模块最好不要屏蔽
     * 注：原来的getDeviceOnline暂不要删掉，因为线程存进对象的数据就是从这个函数取的
     */
    static bool getDeviceOnlineByObject(int sysId, int deviceId, int extenId);
    static bool getDeviceOnlineByObject(int deviceId);
    static bool getDeviceOnlineByObject(DeviceID deviceID);

    // 设备是否是本控
    static bool getDeviceSelfControl(int sysId, int deviceId, int extenId,
                                     int unitId = 1);  // 默认本分控是在单元1里面的 S频段射频开关网络中三个设备是三个不同单元
    static bool getDeviceSelfControl(int deviceId, int unitId = 1);
    static bool getDeviceSelfControl(const DeviceID& deviceID, int unitId = 1);

    // 获取在线设备模式
    static int getOnlineDeviceModeId(int sysId, int deviceId, int extenId);
    static int getOnlineDeviceModeId(const DeviceID& deviceID);
    /*
     * 从平台对象获取在线设备模式,获取速度比redis更快  其中平台对象里面的数据来源于主界面的一个线程处理里,所以主界面模块最好不要屏蔽
     * 注：原来的getOnlineDeviceModeId暂不要删掉，因为线程存进对象的数据就是从这个函数取的
     */
    static int getOnlineDeviceModeIdByObject(int sysId, int deviceId, int extenId);
    static int getOnlineDeviceModeIdByObject(DeviceID deviceID);

    /* 设置和获取当前用户 */
    static void setCurrentUser(const User& currentuser);
    static User getCurrentUser();
    /* 设置和获取当前用户组 */
    static void setUserList(const UserList& userlist);
    static bool getUserList(UserList& userlist);

    /*各个模块在进行相应的操作时都会进入这个函数进行权限判断处理*/
    static bool checkUserLimits(QString& errorMsg);

    // 设置和获取是否是自动运行任务
    static void setAutoRunTaskFlag(bool isAutoRunTask);
    static bool getAutoRunTaskFlag();

    // 设置和获取时间
    static void setTimeData(const TimeData& data);
    static TimeData getTimeData();

    //设置名字控件的Alignment
    static void setDeviceControlAlignment(QLabel* label);

    static QDateTime currentDateTime();
    static QDate currentDate();
    static QTime currentTime();

    /* 设置和获取 服务器在线状态 */
    static bool getServiceOnlineStatus();

    /* 清空和获取当前的任务运行信息 */
    static void cleanTaskRunningInfo();
    static ManualMessage getTaskRunningInfo();
    static ManualMessage getTaskRunningInfoByObject();

    /* 主备机 */
    //static bool masterSlave();
    static HeartbeatMessage masterSlaveInfo();
    static void setMasterSlaveInfo(const HeartbeatMessage&);
};

#endif  // GLOBALDATA_H
