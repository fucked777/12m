#ifndef KAKUO2ZEROCALIBRATIONHANDLER_H
#define KAKUO2ZEROCALIBRATIONHANDLER_H

#include "BaseZeroCalibrationHandler.h"
#include <QObject>

class KaKuo2ZeroCalibrationHandler : public BaseZeroCalibrationHandler
{
public:
    explicit KaKuo2ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode);

    bool handle() override;
};

#endif  // KAKUO2ZEROCALIBRATIONHANDLER_H
