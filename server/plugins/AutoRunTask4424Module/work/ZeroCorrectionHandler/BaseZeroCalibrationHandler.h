#ifndef BASEZEROCALIBRATIONHANDLER_H
#define BASEZEROCALIBRATIONHANDLER_H

#include <QObject>

#include "BusinessMacroCommon.h"
#include "DeviceProcessMessageSerialize.h"
#include "TaskRunCommonHelper.h"

class BaseZeroCalibrationHandler : public QObject
{
    Q_OBJECT
public:
    explicit BaseZeroCalibrationHandler(SystemWorkMode pSystemWorkMode, QObject* parent = nullptr);

    void setTaskTimeList(const TaskTimeList& taskTimeList);
    virtual bool handle() = 0;

    // 执行命令，并且等待命令响应
    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 5);

protected:
    void notifyInfoLog(const QString& logStr);     //普通日志
    void notifyWarningLog(const QString& logStr);  //警告日志
    void notifyErrorLog(const QString& logStr);    //错误日志

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);

protected:
    TaskTimeList mTaskTimeList;
    SystemWorkMode mSystemWorkMode;
};

#endif  // BASEZEROCALIBRATIONHANDLER_H
