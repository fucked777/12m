#ifndef KT_ZeroCalibrationHandler_H
#define KT_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class KT_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit KT_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // KT_ZeroCalibrationHandler_H
