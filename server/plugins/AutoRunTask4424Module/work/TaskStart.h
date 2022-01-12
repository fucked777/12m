#ifndef TASKSTARTEVENT_H
#define TASKSTARTEVENT_H

#include "BaseEvent.h"

struct TaskTimeList;
class TaskStart : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TaskStart(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的
    /**
     * @brief doSomething
     * 实现开始时间判定，循环等待事件开始执行
     * 实现开始时间判定，将不同的计划任务进行装订 然后输出到下一个时间
     */
    void doSomething();
    // 基带开启存盘 送数
    void deviceTaskStart(TaskTimeList taskTimeList);
    // ACU 任务开始
    void acuTaskStart(TaskTimeList taskTimeList);
    // DTE 任务开始
    void dteTaskStart(TaskTimeList taskTimeList);

signals:

public slots:
private:
    bool isRunning = true;
    int leadTime = 5 * 60;  //提前5分钟

    QMap<QString, bool> m_simpleMap = { { "左旋", true }, { "LHCP", true }, { "右旋", true }, { "RHCP", true } };
    QMap<QString, bool> m_doubleMap = { { "HLP", true }, { "左右旋同时", true }, { "VLP", true } };
};

#endif  // STARTCLASS_H
