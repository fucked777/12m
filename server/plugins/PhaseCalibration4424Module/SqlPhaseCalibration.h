#ifndef SQLPHASECALIBRATION_H
#define SQLPHASECALIBRATION_H

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

    // 对外接口 待定 20210621 wp??
    // static Optional<PhaseCalibrationItem> selectItem(const QString& taskCode, double downFreq, int workMode, QList<QVariantMap>&, QString& errMsg);
    // static Optional<PhaseCalibrationItem> insertItem(const QString& taskCode, double downFreq, int workMode, const QVariantMap&, QString& errMsg);
};

#endif  // SQLPHASECALIBRATION_H
