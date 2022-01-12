#ifndef PHASECALIBRATIONSERVICE_H
#define PHASECALIBRATIONSERVICE_H

#include <QList>
#include <QObject>
#include <QThread>

struct CmdResult;
struct DeviceID;
struct PackCommand;
struct ProtocolPack;
struct ManualMessage;
class SingleCommandHelper;
class PhaseCalibration;
class PhaseCalibrationServiceImpl;
class PhaseCalibrationService : public QThread
{
    Q_OBJECT
    friend PhaseCalibrationServiceImpl;

public:
    enum class ExecStatus
    {
        Success,
        Timeout,
        Failed,
        Exit,
    };
    PhaseCalibrationService(PhaseCalibration* parent = nullptr);
    ~PhaseCalibrationService();
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

private:
    void pushLog(const QString& msg, bool error = false);          /* 推送日志 */
    void sendByteArrayToDevice(const QByteArray& data);            /* 发送命令到设备 */
    /* 等待指令执行完成 */
    ExecStatus waitExecSuccess(const PackCommand& packCommand, qint32 ttl = 40);

    void run();

private:
    PhaseCalibrationServiceImpl* m_impl;
};

#endif  // PHASECALIBRATIONSERVICE_H
