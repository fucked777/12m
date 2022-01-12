#ifndef CALIBRATIONEVENT_H
#define CALIBRATIONEVENT_H

#include "AutoRunTaskMessageDefine.h"
#include "BaseEvent.h"
#include <QObject>
#include <QScopedPointer>

class CalibrationImpl;
class Calibration : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit Calibration(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();
};
// Q_DECLARE_METATYPE(Calibration)
#endif  // STARTCLASS_H
