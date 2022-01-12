#ifndef SQLAGC_H
#define SQLAGC_H

#include "Utility.h"
#include <QList>
#include <QObject>

class QSqlDatabase;
struct AGCCalibrationItem;
using AGCCalibrationItemList = QList<AGCCalibrationItem>;
struct AGCResultQuery;
struct ClearAGCCalibrationHistry;
class SqlAGC
{
public:
    SqlAGC();
    ~SqlAGC();
    OptionalNotValue initDB();
    /* 自动化测试 */
    /* 界面上点击保存 */
    Optional<AGCCalibrationItem> insertAGCItem(const AGCCalibrationItem& item);
    Optional<AGCCalibrationItem> updateAGCItem(const AGCCalibrationItem& item);
    Optional<AGCCalibrationItem> saveAGCItem(const AGCCalibrationItem& item);
    Optional<AGCCalibrationItem> deleteAGCItem(const AGCCalibrationItem& item);
    OptionalNotValue deleteAGCItem(const ClearAGCCalibrationHistry& item);
    Optional<AGCCalibrationItemList> selectAGCItem(const AGCResultQuery& cond);

private:
    Optional<AGCCalibrationItem> insertAGCItem(const AGCCalibrationItem& item, QSqlDatabase& db);
    Optional<AGCCalibrationItem> updateAGCItem(const AGCCalibrationItem& item, QSqlDatabase& db);
    bool dataExist(const AGCCalibrationItem& item, QSqlDatabase& db);

private:
    QString m_dbConnectName;
};

#endif  // SQLAGC_H
