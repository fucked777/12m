#ifndef WX_K2_ZeroCalibrationHandler_H
#define WX_K2_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class WX_K2_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit WX_K2_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // WX_K2_ZeroCalibrationHandler_H
