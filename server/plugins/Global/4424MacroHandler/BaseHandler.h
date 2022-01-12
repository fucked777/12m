#ifndef BASEHANDLER_H
#define BASEHANDLER_H

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
    int curModeIDMaster{ -1 };
    DeviceID beforeMaster; /* 切换模式之前的ID */
    DeviceID afterMaster;  /* 切换之后的ID */

    int curModeIDSlave{ -1 };
    DeviceID beforeSlave; /* 切换模式之前的ID */
    DeviceID afterSlave;  /* 切换之后的ID */
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
    bool getGSJDDeviceID(DeviceID& deviceID, int no = 1);
    void getGSJDDeviceIDS(QList<DeviceID>& deviceIDList);

    // 获取当前链路使用第几台低速基带设备ID
    bool getDSJDDeviceID(DeviceID& deviceID, int no = 1);
    void getDSJDDeviceIDS(QList<DeviceID>& deviceIDList);

    // 获取当前链路使用的主用跟踪基带设备ID
    bool getMasterGZJDDeviceID(DeviceID& deviceID);

    // 获取当前链路使用的测控前端设备ID
    bool getCKQDDeviceID(DeviceID& deviceID);
    // 获取当前链路使用的跟踪前端设备ID
    bool getGZQDDeviceID(DeviceID& deviceID);

    // 获取当前链路使用的S高功放设备ID
    bool getSGGFDeviceID(DeviceID& deviceID);
    // 获取当前链路使用的Ka测控高功放设备ID
    bool getKaGGFDeviceID(DeviceID& deviceID);
    // 获取当前链路使用的Ka数传高功放设备ID
    bool getKaDtGGFDeviceID(DeviceID& deviceID);

    // 获取当前链路使用的X1.2GHz下变频器设备ID(4424)
    bool getXXBPQDeviceID(DeviceID& deviceID, int no = 1);
    // 获取当前链路使用的Ka1.2GHz下变频器设备ID(4424)
    bool getKaXBPQDeviceID(DeviceID& deviceID, int no = 1);

    // 获取当前链路使用的Ka高速下变频器设备ID（4426）
    bool getKaGSXBPQDeviceID(DeviceID& deviceID, int& unitId, int no = 1);

protected:
    // 执行命令，并且等待命令响应
    bool waitExecSuccess(const PackCommand& packCommand, qint32 ttl = 10);

    void appendExecQueue(const PackCommand& packCommand);
    void execQueue(int ttl = 5);

    enum class CKJDSwitchStatus
    {
        All,     // 全部切换成功
        Master,  // 主机切换成功
        Slave,   // 备机切换成功
        Failed,  // 全部切换失败
    };

    // 切换基带的工作模式
    bool switchJDWorkMode(const DeviceID& deviceID, SystemWorkMode needWorkMode);
    // 切换跟踪基带的工作模式
    bool switchGZJDWorkMode(const DeviceID& deviceID, int needRealModeId);

    // 切换4424测控基带主备机
    bool switchCKJDMasterSlave();
    // 切换4426测控基带主备机
    bool switchCKJDMasterSlave(const CKJDControlInfo& info);

    // 切换高速基带主备机
    bool switchGSJDMasterSlave();
    // 切换低速基带主备机
    bool switchDSJDMasterSlave();
    // 切换跟踪基带主备机
    bool switchGZJDMasterSlave();

    // 基带主备切换
    void switchJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave);
    // 高速基带不合群他的主备切换命令不一样
    void switchGSJDMasterSlave(const DeviceID& deviceID, MasterSlave masterSlave);

    /* 退出标志 */
    bool isRunning();
    bool isExit();

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);

private slots:
    // 收到控制命令响应
    void slotControlCmdResponse(const ControlCmdResponse& cmdResponse);

protected:
    /* 退出标志 */
    std::atomic<bool>* mRunningFlag;

    ManualMessage mManualMsg;
    LinkLine mLinkLine;  // 当前链路信息

    QList<PackCommand> mPackCommandQueue;  // 待执行的命令队列
    QList<PackCommand> mSentCommandList;   // 记录已发送的命令

    SingleCommandHelper mSingleCommandHelper; /* 命令组包 */
    PackCommand mPackCommand;
};

#endif  // BASEHANDLER_H
