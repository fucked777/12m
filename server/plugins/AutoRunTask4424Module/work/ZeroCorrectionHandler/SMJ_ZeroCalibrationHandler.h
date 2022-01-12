#ifndef SMJ_ZeroCalibrationHandler_H
#define SMJ_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class SMJ_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit SMJ_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // SMJ_ZeroCalibrationHandler_H
