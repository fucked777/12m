#ifndef GOUP_H
#define GOUP_H

#include "BaseEvent.h"
class GoUp : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit GoUp(QObject* parent = nullptr);

    static int typeId;  //实现反射使用的

    void doSomething();
signals:

public slots:
};
// Q_DECLARE_METATYPE(GoUp)

#endif  // GOUP_H
