#ifndef BASESTATUS_H
#define BASESTATUS_H

#include <QObject>

struct BaseStatus
{
    BaseStatus(const QString& currentState, const QString& currentClass, const QString& nextState)
    {
        this->currentState = currentState;
        this->currentClass = currentClass;
        this->nextState = nextState;
    }
    QString currentState;
    QString currentClass;
    QString nextState;
};

#endif  // BASESTATUS_H
