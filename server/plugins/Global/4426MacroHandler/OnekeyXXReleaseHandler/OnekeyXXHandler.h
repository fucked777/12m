#ifndef ONEKEYXXHANDLER_H
#define ONEKEYXXHANDLER_H

#include "BaseHandler.h"
#include <QScopedPointer>

// 资源释放处理
class OnekeyXXHandlerImpl;
class OnekeyXXHandler : public BaseHandler
{
    Q_OBJECT
public:
    explicit OnekeyXXHandler(QObject* parent = nullptr);
    ~OnekeyXXHandler();

    bool handle() override;

private:
    void calibrationError(const QString& logStr);
    void addXuanXiang(QVariantMap& acuCalibrationParamMap);

signals:
    /* 音频推送 */
    void sg_audio(const QString&);
    void signalSendData(const QByteArray& data);

private:
    // 还原任务链路
    ACUPhaseCalibrationFreqBand trackingModeToAcuFreqBandStatus(SatelliteTrackingMode mode);

private:
    QScopedPointer<OnekeyXXHandlerImpl> m_impl;
};

#endif  // ONEKEYXXHANDLER_H
