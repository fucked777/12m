#ifndef SqlSystemLogManager_H
#define SqlSystemLogManager_H

#include "Utility.h"
#include <QObject>

class QString;
struct SystemLogData;
struct SystemLogACK;
class SqlSystemLogManager : public QObject
{
    Q_OBJECT
public:
    explicit SqlSystemLogManager(QObject* parent = nullptr);
    ~SqlSystemLogManager();

public:
    OptionalNotValue initDB();

    //有时间的话，就补上按时间批量删除日志的接口
    Optional<int> totalNumberOfQueries(quint64 beginTime, quint64 endTime, int logLevel);
    /*
     * currentPage 从0开始
     */
    Optional<SystemLogACK> logConditionQuery(quint64 beginTime, quint64 endTime, int logLevel, int currentPage, int pageSize);
    void insertSysLog(const SystemLogData& pSystemLog);
    void insertSysLog(const QList<SystemLogData>& pSystemLog);

private:
    QString mDbConnectName;
};

#endif  // SqlSystemLogManager_H
