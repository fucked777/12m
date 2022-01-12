#ifndef ACUPARAMMACROHANDLER_H
#define ACUPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"
#include "SatelliteManagementDefine.h"

// ACU参数宏
class ACUParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit ACUParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    // 获取当前ACU的跟踪模式
    bool getTrackingMode();

private:
    SatelliteManagementData m_satelliteTemp;
    SatelliteTrackingMode m_trackingMode { SatelliteTrackingMode::TrackUnknown };
    int m_trackingPointFreqNo { -1 };
};
#endif  // ACUPARAMMACROHANDLER_H
