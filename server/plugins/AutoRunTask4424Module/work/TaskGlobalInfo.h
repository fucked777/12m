#ifndef TaskGlobalInfo_H
#define TaskGlobalInfo_H

#include "AutoRunTaskMessageDefine.h"
#include "TaskRunCommonHelper.h"

#define ORIGINALFORMAT "yyyyMMddHHmmss"
#define TimeZero       QString("00000000000000")
#define TimeF          QString("FFFFFFFFFFFFFF")

class TaskGlobalInfoSingleton
{
    virtual ~TaskGlobalInfoSingleton() {}

public:
    static TaskGlobalInfoSingleton& getInstance()
    {
        static TaskGlobalInfoSingleton instance;

        return instance;
    }

    TaskGlobalInfoSingleton(const TaskGlobalInfoSingleton&) = delete;
    TaskGlobalInfoSingleton& operator=(const TaskGlobalInfoSingleton&) = delete;

private:
    TaskGlobalInfoSingleton() = default;

public:
public:
    void instanceData();

    void setTaskTimeList(QString uuid, TaskTimeList list) { m_taskTimeMap[uuid] = list; }
    TaskTimeList taskTimeList(QString uuid) { return m_taskTimeMap[uuid]; }
    /* 20210419 添加一个接口 默认获取第一个数据 */
    TaskTimeList taskTimeList() { return m_taskTimeMap.isEmpty() ? TaskTimeList() : m_taskTimeMap.first(); }

    CalibResultInfo getCalibResultInfo(int workMode, const QString& taskCode, double workFreg);
    void updateCalibResultInfoList(CalibResultInfo info);
    QList<CalibResultInfo> getCalibResulitList() const;

private:
    QMap<QString, TaskTimeList> m_taskTimeMap;

    QList<CalibResultInfo> m_calibResulitList;
};

#endif  // TaskGlobalInfo_H
