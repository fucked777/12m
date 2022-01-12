#ifndef SKUO2ZEROCALIBRATIONHANDLER_H
#define SKUO2ZEROCALIBRATIONHANDLER_H

#include "BaseZeroCalibrationHandler.h"
#include <QObject>

class SKuo2ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit SKuo2ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // SKUO2ZEROCALIBRATIONHANDLER_H
