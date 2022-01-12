#ifndef REPEATALARMINTERVAL_H
#define REPEATALARMINTERVAL_H

#include <QDialog>

namespace Ui
{
    class RepeatAlarmInterval;
}

class RepeatAlarmInterval : public QDialog
{
    Q_OBJECT

public:
    explicit RepeatAlarmInterval(qint32 curInterval, QWidget* parent = nullptr);
    ~RepeatAlarmInterval();
    qint32 interval();

private:
    Ui::RepeatAlarmInterval* ui;
};

#endif  // REPEATALARMINTERVAL_H
