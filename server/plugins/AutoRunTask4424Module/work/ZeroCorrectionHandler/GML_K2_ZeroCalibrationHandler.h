#ifndef GML_K2_ZeroCalibrationHandler_H
#define GML_K2_ZeroCalibrationHandler_H
#include "BaseZeroCalibrationHandler.h"

class GML_K2_ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit GML_K2_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // STTCZEROCALIBRATIONHANDLER_H
