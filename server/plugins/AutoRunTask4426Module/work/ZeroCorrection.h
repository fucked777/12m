#ifndef ZEROCORRECTIONEVENT_H
#define ZEROCORRECTIONEVENT_H

#include "BaseEvent.h"
#include "TaskRunCommonHelper.h"

#include <QObject>
class ZeroCorrection : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ZeroCorrection(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();

signals:

public slots:

private:
    void acu2Point(const TaskTimeList& taskTimeList);
};
// Q_DECLARE_METATYPE(ZeroCorrection)

#endif  // ZEROCORRECTIONEVENT_H
