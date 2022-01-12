#ifndef WX_ZeroCalibrationHandler_H
#define WX_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class WX_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit WX_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // WX_ZeroCalibrationHandler_H
