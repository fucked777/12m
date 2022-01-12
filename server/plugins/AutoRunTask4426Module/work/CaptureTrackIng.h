#ifndef CAPTURETRACKING_H
#define CAPTURETRACKING_H
#include "BaseEvent.h"

class CaptureTrackIng : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit CaptureTrackIng(QObject* parent = nullptr);
    virtual ~CaptureTrackIng();

    static int typeId;  //实现反射使用的

    void doSomething();
signals:

public slots:
private:
};
// Q_DECLARE_METATYPE(CaptureTrackIng)

#endif  // CAPTURETRACKING_H
