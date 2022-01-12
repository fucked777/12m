#ifndef TRACKEND_H
#define TRACKEND_H

#include "BaseEvent.h"
struct TaskTimeList;
class TrackEnd : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TrackEnd(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();
};
// Q_DECLARE_METATYPE(TrackEnd)
#endif  // TRACKEND_H
