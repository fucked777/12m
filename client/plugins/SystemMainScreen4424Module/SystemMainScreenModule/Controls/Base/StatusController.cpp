#include "StatusController.h"

#include "StatusTimer.h"

StatusController::StatusController()
    : mStatus(Status::Unknown)
    , mTimer(new StatusTimer(this))
{
}

StatusController::~StatusController()
{
    delete mTimer;
    mTimer = nullptr;
}

StatusController::Status StatusController::getStatus() const { return mStatus; }

void StatusController::setStatus(StatusController::Status status)
{
    if (mStatus != status)
    {
        statusChanged(status);
    }

    mStatus = status;
    if (mStatus != Unknown)
    {
        mTimer->start(8000);
    }
}

void StatusController::statusChanged(Status status) { Q_UNUSED(status) }
