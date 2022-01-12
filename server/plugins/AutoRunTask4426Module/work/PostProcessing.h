#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include "BaseEvent.h"
class PostProcessing : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit PostProcessing(QObject* parent = nullptr);

    static int typeId;  //实现反射使用的

    void doSomething();
signals:

public slots:
};
// Q_DECLARE_METATYPE(PostProcessing)
#endif  // POSTPROCESSING_H
