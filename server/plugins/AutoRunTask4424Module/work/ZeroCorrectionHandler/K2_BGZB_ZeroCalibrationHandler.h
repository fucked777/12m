#ifndef K2_BGZB_ZeroCalibrationHandler_H
#define K2_BGZB_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class K2_BGZB_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit K2_BGZB_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // K2_BGZB_ZeroCalibrationHandler_H
