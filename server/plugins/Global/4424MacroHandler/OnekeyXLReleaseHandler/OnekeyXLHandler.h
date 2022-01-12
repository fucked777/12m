#ifndef ONEKEYXLHANDLER_H
#define ONEKEYXLHANDLER_H

#include "BaseHandler.h"

class OnekeyXLHandlerImpl;
class OnekeyXLHandler : public BaseHandler
{
    Q_OBJECT
public:
    explicit OnekeyXLHandler(QObject* parent = nullptr);
    ~OnekeyXLHandler();

    void setRestoreRWLink(bool restore, SystemWorkMode upWorkMode);
    bool handle() override;
    bool getResult(double& distZeroMean);

private:
    bool waitCalibrationResult(const PackCommand& pack);
    bool STTC_ZeroCalibration(DeviceID ckDeviceID);           // STTC校零
    bool SKuo2_ZeroCalibration(DeviceID ckDeviceID);          // SKuo2校零
    bool SYTHSMJ_ZeroCalibration(DeviceID ckDeviceID);        // 一体化上面级
    bool SYTHWX_ZeroCalibration(DeviceID ckDeviceID);         // 一体化卫星
    bool SYTHGML_ZeroCalibration(DeviceID ckDeviceID);        // 一体化高码率
    bool SYTHSMJK2GZB_ZeroCalibration(DeviceID ckDeviceID);   // 一体化上面级+扩二共载波
    bool SYTHSMJK2BGZB_ZeroCalibration(DeviceID ckDeviceID);  // 一体化上面级+扩二不共载波
    bool SYTHWXSK2_ZeroCalibration(DeviceID ckDeviceID);      // 一体化卫星+扩二
    bool SYTHGMLSK2_ZeroCalibration(DeviceID ckDeviceID);     // 一体化高码率+扩二
    bool SKT_ZeroCalibration(DeviceID ckDeviceID);            // 扩跳

signals:
    void signalSendData(const QByteArray& data);

private:
    double m_distZeroMean = 0;
    bool m_isSuccess = false;

    QScopedPointer<OnekeyXLHandlerImpl> mImpl;
};

#endif  // OnekeyXLHandler_H
