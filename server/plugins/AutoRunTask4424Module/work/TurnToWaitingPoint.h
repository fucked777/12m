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
};

#endif  // TURNTOWAITINGPOINT_H
