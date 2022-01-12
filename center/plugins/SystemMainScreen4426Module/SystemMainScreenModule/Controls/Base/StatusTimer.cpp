#include "StatusTimer.h"
#include "StatusController.h"

StatusTimer::StatusTimer(StatusController* statusController)
{
    mStatusController = statusController;
    setSingleShot(true);
}

void StatusTimer::timerEvent(QTimerEvent*)
{
    if (mStatusController != nullptr)
    {
        mStatusController->setStatus(StatusController::Unknown);
    }
}
