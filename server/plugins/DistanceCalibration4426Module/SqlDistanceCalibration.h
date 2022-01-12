#ifndef SQLDISTANCECALIBRATION_H
#define SQLDISTANCECALIBRATION_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>

struct DistanceCalibrationItem;
using DistanceCalibrationItemList = QList<DistanceCalibrationItem>;

struct QueryDistanceCalibrationHistry;
struct ClearDistanceCalibrationHistry;
class SqlDistanceCalibration : public QObject
{
public:
    static OptionalNotValue initDB();
    static Optional<DistanceCalibrationItem> insertItem(const DistanceCalibrationItem& item);
    static Optional<DistanceCalibrationItem> deleteItem(const DistanceCalibrationItem& item);
    static OptionalNotValue deleteItem(const ClearDistanceCalibrationHistry& item);
    static Optional<DistanceCalibrationItemList> selectItem(const QueryDistanceCalibrationHistry&);
};

#endif  // SQLDISTANCECALIBRATION_H
