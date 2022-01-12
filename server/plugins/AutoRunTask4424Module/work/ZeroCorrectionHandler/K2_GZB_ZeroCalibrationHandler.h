#ifndef K2_GZB_ZeroCalibrationHandler_H
#define K2_GZB_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class K2_GZB_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit K2_GZB_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // K2_GZB_ZeroCalibrationHandler_H
