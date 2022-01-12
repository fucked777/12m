#ifndef ONEKEYXLHANDLER_H
#define ONEKEYXLHANDLER_H

#include "BaseParamMacroHandler.h"
#include <QScopedPointer>

// 资源释放处理
class OnekeyXLHandlerImpl;
class OnekeyXLHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit OnekeyXLHandler(QObject* parent = nullptr);
    ~OnekeyXLHandler();

    SystemWorkMode getSystemWorkMode() override;
    bool handle() override;
    bool getResult(double& distZeroMean);

private:
    bool waitCalibrationResult(SystemWorkMode workMode, const PackCommand& pack);

    bool STTC_ZeroCalibration(const DeviceID& ckjdDeviceID);      // STTC校零
    void STTC_ZeroCalibration_End(const DeviceID& ckjdDeviceID);  // STTC校零

    bool SKuo2_ZeroCalibration(const DeviceID& ckjdDeviceID);      // SKuo2校零
    void SKuo2_ZeroCalibration_End(const DeviceID& ckjdDeviceID);  // SKuo2校零

    bool KaKuo2_ZeroCalibration(const DeviceID& ckjdDeviceID);      // Kak2校零
    void KaKuo2_ZeroCalibration_End(const DeviceID& ckjdDeviceID);  // Kak2校零

    bool SKT_ZeroCalibration(const DeviceID& ckjdDeviceID);      // 扩跳
    void SKT_ZeroCalibration_End(const DeviceID& ckjdDeviceID);  // 扩跳

signals:
    void signalSendData(const QByteArray& data);

private:
    // 还原任务链路
    void restoreRWLink(const DeviceID& ckjdDeviceID);
    void jdParamRestore(const DeviceID& ckjdDeviceID);

private:
    QScopedPointer<OnekeyXLHandlerImpl> m_impl;
};

#endif  // OnekeyXLHandler_H
