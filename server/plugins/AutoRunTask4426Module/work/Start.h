#ifndef STARTEVENT_H
#define STARTEVENT_H
#include "BaseEvent.h"
#include <QObject>
class Start : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit Start(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();
signals:

public slots:
};
// Q_DECLARE_METATYPE(Start)
#endif  // STARTEVENT_H
