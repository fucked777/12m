#ifndef ZEROCALIBRATIONROUTOR_H
#define ZEROCALIBRATIONROUTOR_H

#include "BaseZeroCalibrationHandler.h"
#include "SystemWorkMode.h"
#include "TaskRunCommonHelper.h"
#include <QObject>
class ZeroCalibrationRoutor : public QObject
{
    Q_OBJECT
public:
    explicit ZeroCalibrationRoutor(QObject* parent = nullptr);

    static BaseZeroCalibrationHandler* factory(const SystemWorkMode systemWorkMode);
};

#endif  // ZEROCALIBRATIONROUTOR_H
