#ifndef STATUSTIMER_H
#define STATUSTIMER_H

#include <QTimer>

// 状态计时器
class StatusController;
class StatusTimer : public QTimer
{
    Q_OBJECT
public:
    explicit StatusTimer(StatusController* statusController);

protected:
    void timerEvent(QTimerEvent*);

private:
    StatusController* mStatusController = nullptr;
};

#endif  // STATUSTIMER_H
