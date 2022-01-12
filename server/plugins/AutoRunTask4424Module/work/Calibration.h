#ifndef CALIBRATIONEVENT_H
#define CALIBRATIONEVENT_H

#include "BaseEvent.h"
#include <QObject>

class Calibration : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit Calibration(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();

private:
    void calibrationError(const QString& logStr);
    void trackTaskStart();
    void programTaskStart();
    void addXuanXiang(QVariantMap&);

private:
    TaskTimeList m_taskTimeList;
};
#endif  // STARTCLASS_H
