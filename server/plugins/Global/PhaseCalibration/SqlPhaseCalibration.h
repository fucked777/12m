#ifndef SQLPHASECALIBRATION_H
#define SQLPHASECALIBRATION_H

#include "SystemWorkMode.h"
#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct ClearPhaseCalibrationHistry;
struct PhaseCalibrationItem;
using PhaseCalibrationItemList = QList<PhaseCalibrationItem>;
struct QueryPhaseCalibrationHistry;
class SqlPhaseCalibration
{
public:
    static OptionalNotValue initDB();
    /* 校相 */
    static Optional<PhaseCalibrationItem> insertItem(const PhaseCalibrationItem& item);
    static Optional<PhaseCalibrationItem> deleteItem(const PhaseCalibrationItem& item);
    static OptionalNotValue deleteItem(const ClearPhaseCalibrationHistry& item);
    static Optional<PhaseCalibrationItemList> selectItem(const QueryPhaseCalibrationHistry&);

    static Optional<PhaseCalibrationItemList> selectItem(const QString& taskCode, int pointFreqNo, SystemWorkMode workMode, quint64 equipComb);
};

#endif  // SQLPHASECALIBRATION_H
