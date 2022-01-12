#ifndef GML_ZeroCalibrationHandler_H
#define GML_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class GML_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit GML_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // GML_ZeroCalibrationHandler_H
