#ifndef BASEHANDLER_H
#define BASEHANDLER_H

#include "AutoRunTaskMessageDefine.h"
#include "BusinessMacroCommon.h"
#include "PlanRunMessageDefine.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SingleCommandHelper.h"
#include <QObject>
#include <QScopedPointer>
#include <atomic>

/* 处理4426SB测控基带单模式还分这么多设备ID */
/* 获取测控基带的各种信息*/
struct CKJDControlInfo
{
    bool afterDoubleMode{ false };        /* 切换之后是否是双模式 */
    bool beforeMasterDoubleMode{ false }; /* 切换之前是否是双模式 */
    int curModeIDMaster{ -1 };
    DeviceID beforeMaster; /* 切换模式之前的ID */
    DeviceID afterMaster;  /* 切换之后的ID */

    bool beforeSlaveDoubleMode{ false }; /* 切换之前是否是双模式 */
    int curModeIDSlave{ -1 };
    DeviceID beforeSlave; /* 切换模式之前的ID */
    DeviceID afterSlave;  /* 切换之后的ID */

    int getNeedSwitchWorkMode() const;
};

struct MasterSlaveDeviceIDInfo
{
    DeviceID master;
    DeviceID slave;
};

// 跟踪基带的模式ID
enum class TBBEModeID
{
    Unknown = 0,
    STTC = 0x01,       //标准TTC
    KP = 0x02,         //扩频TTC
    KaGS = 0x04,       //高速数传跟踪
    STTC_KaGS = 0x05,  //标准TTC+高速数传跟踪
    KP_KaGS = 0x06,    //扩频TTC+高速跟踪
    SKT = 0x09,        //扩跳频跟踪
    STTC_KaDS = 0x0A,  //标准TTC+低速跟踪
    KP_KaDS = 0x0B,    //扩频TTC+低速跟踪
    KaDS = 0x0C        //低速数传跟踪
};

class BaseHandler : public QObject
{
    Q_OBJECT
public:
    explicit BaseHandler(QObject* parent = nullptr);

    virtual bool handle() = 0;

    void setManualMessage(const ManualMessage& manualMsg);
    /* 用于退出 */
    void setRunningFlag(std::atomic<bool>* runningFlag);

    // 设置当前链路
    void setLinkLine(const LinkLine& linkLine);

    // 获取主用测控基带数量
    bool getCKJDMasterNumber(int& number);
    // 获取主用高速基带数量
    bool getGSJDMasterNumber(int& number);
    // 获取主用低速基带数量
    bool getDSJDMasterNumber(int& number);

    // 获取当前链路使用第几台测控基带设备ID
    bool getCKJDDeviceID(DeviceID& deviceID, int no = 1);

    // 获取当前链路使用第几台高速基带设备ID
    Optional<MasterSlaveDeviceIDInfo> getGSJDDeviceID();
    void getGSJDDeviceIDS(QList<DeviceID>& deviceIDList);

    // 获取当前链路使用第几台低速基带设备ID
    Optional<MasterSlaveDeviceIDInfo> getDSJDDeviceID();
    void getDSJDDeviceIDS(QList<DeviceID>& deviceIDList);

    // 获取当前链路使用的主用跟踪基带设备ID
    bool getMasterGZJDDeviceID(DeviceID& deviceID);

    // 获取当前链路使用的测控前端设备ID
    Optional<MasterSlaveDeviceIDInfo> getCKQDDeviceID();
    // 获取当前链路使用的跟踪前端设备ID
    Optional<MasterSlaveDeviceIDInfo> getGZQDDeviceID();

    // 获取当前链路使用的S高功放设备ID
    bool getSGGFDeviceID(DeviceID& deviceID);
    // 获取当前链路使用的Ka测控高功放设备ID
    bool getKaGGFDeviceID(DeviceID& deviceID);
    // 获取当前链路使用的Ka数传高功放设备ID
    bool getKaDtGGFDeviceID(DeviceID& deviceID);

    // 测控基带双模式的ID
    static QString getCKJDWorkModeDesc(SystemWorkMode needWorkMode, bool isDoubleMode);
    static int getCKJDDoubleModeID();
    bool isDoubleMode() const;

    // 转换跟踪基带模式
    TBBEModeID convertGZJDMode(const QSet<SystemWorkMode>&);
    // 获取备用跟踪基带ID
    static DeviceID getGZJDSlaveDeviceID(const DeviceID& deviceID);
    /*
     * 测控基带 4001/4003 S  4002/4004 ka
     */
    // 获取双模式的另一个模式的ID
    static DeviceID getEqModeCKDeviceID(const DeviceID& deviceID);
    // 获取当前设备ID对应另一台测控设备的ID
    static DeviceID getNextCKDeviceID(const DeviceID& deviceID);
    // 获取双模式在当前设备上的设备ID
    static DeviceID getChangeModeCKDeviceID(const DeviceID& deviceID, SystemWorkMode workMode);
    // 获取主跟任务代号
    static Optional<QString> getMasterTrackingTaskCode(const ManualMessage& mManualMsg);
    // 跟踪模式和系统模式转换 只会转换成最终的跟踪模式
    static SystemWorkMode trackingToSystemWorkMode(SatelliteTrackingMode mode);
    static QList<SystemWorkMode> trackingToSystemWorkModeList(SatelliteTrackingMode mode);
    /* 获取ACU的系统工作方式 */
    static Optional<int> getACUSystemWorkMode(SatelliteTrackingMode mode);
    /* 获取当前在线(主机)ACU */
    static Optional<DeviceID> getOnlineACU();
    // 当前下发的任务模式和设置的跟踪模式对应不上时需要处理一下
    static Optional<std::tuple<SatelliteTrackingMode, int>> checkTrackingInfo(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingMode);
    static Optional<SatelliteTrackingMode> checkTrackingMode(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingMode);
    static Optional<int> checkTrackingPointFreq(const ManualMessage& mManualMsg, SatelliteTrackingMode trackingMode);
    static Optional<LinkType> manualMessageTaskCheckOperation(const ManualMessage&);
    /* 切换之前获取基带信息
     * 这里主用基带获取失败会报错,备用基带获取失败会有错误信息但是不会错误
     */
    static Optional<CKJDControlInfo> getCKJDControlInfo(const ConfigMacroData& configMacroData, SystemWorkMode needWorkMode);

    /* 标校结果保存与查询 */
    static CalibResultInfo getCalibResultInfo(const QString& taskCode, int dotDrequency, SystemWorkMode workMode, quint64 equipComb,
                                              bool defaultValue = false);
    static void updateCalibResultInfoList(const CalibResultInfo& info);

    /* 获取当前的站ID */
    static Optional<QString> getStationCKUAC();
protected:
    // 执行命令，并且等待命令响应
    bool waitExecSuccess(const PackCommand& packCommand, qint32 ttl = 10);

    void clearQueue();
    void appendExecQueue(const PackCommand& packCommand);
    void execQueue(int ttl = 15);

    // 测控基带的切换状态(主备)
    enum class CKJDSwitchStatus
    {
        All,     // 全部切换成功
        Master,  // 主机切换成功
        Slave,   // 备机切换成功
        Failed,  // 全部切换失败
    };

    /* 切换之前获取基带信息
     * 这里主用基带获取失败会报错,备用基带获取失败会有错误信息但是不会错误
     */
    Optional<CKJDControlInfo> getCKJDControlInfo(SystemWorkMode needWorkMode);
    // 切换测控基带的工作模式 主备都切
    //
    CKJDSwitchStatus switchJDWorkMode(const CKJDControlInfo& deviceInfo, SystemWorkMode needWorkMode);
    //  切换单个基带 等待结束
    bool switchJDWorkMode(const CKJDControlInfo& deviceInfo, SystemWorkMode needWorkMode, bool master);
    // 切换跟踪基带的工作模式
    bool switchGZJDWorkMode(const DeviceID& deviceID, int needRealModeId);

    //  切换单个基带 不等待 返回指令是否下发
    bool switchGZJDWorkModeNotWait(const DeviceID& deviceID, int needSwitchWorkMode);
    // 切换跟踪基带的工作模式 主备一起切换  这里用下测控基带的枚举
    CKJDSwitchStatus switchGZJDMasterAndSlaveWorkMode(const DeviceID& masterDeviceID, const DeviceID& slaveDeviceID, int needSwitchModeId);

    // 切换4424测控基带主备机
    // bool switchCKJDMasterSlave();
    // 切换4426测控基带主备机
    bool switchCKJDMasterSlave(const CKJDControlInfo& info);

    // 切换高速基带主备机
    bool switchGSJDMasterSlave();
    bool switchGSJDMasterSlave(const MasterSlaveDeviceIDInfo& info);
    // 切换低速基带主备机
    bool switchDSJDMasterSlave();
    bool switchDSJDMasterSlave(const MasterSlaveDeviceIDInfo& info);
    // 切换跟踪基带主备机
    bool switchGZJDMasterSlave();

    // 基带主备切换
    void switchJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave);
    // 高速基带不合群他的主备切换命令不一样
    void switchGSJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave);

    /* 清空DTE任务 */
    void cleanDTETask();
    /* 退出标志 */
    bool isRunning();
    bool isExit();
#define ExitCheck(v)                                                                                                                                 \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (isExit())                                                                                                                                \
        {                                                                                                                                            \
            return v;                                                                                                                                \
        }                                                                                                                                            \
    } while (0)
#define ExitCheckVoid()                                                                                                                              \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (isExit())                                                                                                                                \
        {                                                                                                                                            \
            return;                                                                                                                                  \
        }                                                                                                                                            \
    } while (0)
private:
    // 单个
    enum class CKJDItemSwitchStatus
    {
        Success,     // 切换成功
        Failed,      // 切换失败
        CmdSuccess,  // 指令下发
    };
    //  切换单个基带 不等待 返回指令是否下发
    CKJDItemSwitchStatus switchJDWorkModeNotWait(const CKJDControlInfo& deviceInfo, SystemWorkMode needWorkMode, bool master);
signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);
    //
    void signalSpecificTipsMsg(const QString& msg);

protected:
    /* 退出标志 */
    std::atomic<bool>* mRunningFlag;

    ManualMessage mManualMsg;
    LinkLine mLinkLine;  // 当前链路信息

    QList<PackCommand> mPackCommandQueue;  // 待执行的命令队列

    //    QList<PackCommand> mSentCommandList;   // 记录已发送的命令
    QMap<int, PackCommand> mSentCommandMap;   // 记录发送的map
    QMap<int, QString> mSentCommandRedisMap;  //记录redis的kay的map

    static SingleCommandHelper mSingleCommandHelper; /* 命令组包 */
    PackCommand mPackCommand;
};

#endif  // BASEHANDLER_H
