#ifndef AGCCALIBRATIONSERVER_H
#define AGCCALIBRATIONSERVER_H
#include "AGCDefine.h"
#include <QList>
#include <QObject>
#include <QThread>

struct CmdResult;
struct DeviceID;
struct PackCommand;
struct ProtocolPack;
struct ManualMessage;
class SingleCommandHelper;
class AGCManage;
class AGCCalibrationServerImpl;
class AGCCalibrationServer : public QThread
{
    Q_OBJECT
    friend AGCCalibrationServerImpl;

public:
    enum class ExecStatus
    {
        Success,
        Timeout,
        Failed,
        Exit,
    };
    AGCCalibrationServer(AGCManage* parent);
    ~AGCCalibrationServer();
    /*
     * 20210806 wp??
     * 这里的停止函数说明
     * 因为此类是调用任务计划的流程
     * 然而任务计划里面未提供比较细节的停止流程
     * 是直接强制杀死进程的
     * 这里更改代码的复杂度很高 于是直接还用任务计划的逻辑
     * 直接强制杀死进程代表停止
     */
    void startServer(const ProtocolPack& pack);
    void stopServer();

signals:
    void sg_end();
    void sg_testProcessInfo(const ProtocolPack&);

private:
    void pushLog(const QString& msg, bool error = false);          /* 推送日志 */
    void acceptDeviceCMDResultMessage(const ProtocolPack& pack);   /* 命令结果 */
    void acceptDeviceCMDResponceMessage(const ProtocolPack& pack); /* 命令响应 */
    void sendByteArrayToDevice(const QByteArray& data);            /* 发送命令到设备 */

    ExecStatus waitExecSuccessImpl(const PackCommand& packCommand, qint32 ttl = 40, bool timeoutError = true);
    /* 等待指令执行完成 */
    ExecStatus waitExecSuccess(const PackCommand& packCommand, qint32 ttl = 40);
    /* 等待状态到位 单个参数 */
    ExecStatus waitSingleStatusChange(const PackCommand& packCommand, const DeviceID& devID, qint32 unitID, const QString& paramID,
                                      const QVariant& value, qint32 ttl = 40);
    /* 等待有结果上报的数据 */
    ExecStatus waitExecResponceData(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count,
                                    qint32 ttl); /* count重试次数 timeout超时时间 */

    void run();

private:
    AGCCalibrationServerImpl* m_impl;
};

#endif  // AGCCALIBRATIONSERVER_H
