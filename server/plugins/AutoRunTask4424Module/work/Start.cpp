#include "Start.h"

#include <QDebug>
#include <QThread>

// int Start::typeId = qRegisterMetaType<Start>();
Start::Start(QObject* parent)
    : BaseEvent(parent)
{
}

void Start::doSomething()
{
    notifyInfoLog("任务等待执行");

    notify("TaskPreparation");
    return;
}
