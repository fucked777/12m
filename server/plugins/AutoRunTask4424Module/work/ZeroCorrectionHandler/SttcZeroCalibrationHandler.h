#ifndef STTCZEROCALIBRATIONHANDLER_H
#define STTCZEROCALIBRATIONHANDLER_H
#include "BaseZeroCalibrationHandler.h"

class SttcZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit SttcZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // STTCZEROCALIBRATIONHANDLER_H
