#ifndef SYSTEMLOGSUBSCRIBER_H
#define SYSTEMLOGSUBSCRIBER_H

#include "SqlSystemLogManager.h"

#include <QObject>

class SystemLogSubscriber : public QObject
{
    Q_OBJECT
public:
    SystemLogSubscriber();

private slots:
    void slotReadSystemLog(const SystemLogData& data);

private:
    SqlSystemLogManager sql;
};

#endif  // SYSTEMLOGSUBSCRIBER_H
