#ifndef TURNTOWAITINGPOINT_H
#define TURNTOWAITINGPOINT_H

#include "BaseEvent.h"

class TurnToWaitingPoint : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TurnToWaitingPoint(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();

private:
    // 查询ACU任务信息
    bool waitTaskPlanInfo(const TaskTimeList& taskTimeList);
    // 等待角度到
    void waitAngle(const TaskTimeList& taskTimeList);

private:
    // 进站方位俯仰
    double m_apprPosit{ 0.0 };
    double m_pitchPitch{ 0.0 };

    // 等待点方位俯仰
    double m_azimWaitPoint{ 0.0 };
    double m_pitchWaitPoint{ 0.0 };
};

#endif  // TURNTOWAITINGPOINT_H
