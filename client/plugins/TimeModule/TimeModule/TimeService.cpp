#include "TimeService.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "TimeMessageSerialize.h"
#include "TimeSyncUDPGB.h"
#include <QThread>

#include <QUdpSocket>

class TimeServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    QThread *thread {nullptr};
    TimeSyncUDPGB *timeSyncUDPGB{nullptr};
};
TimeService::TimeService(cppmicroservices::BundleContext context)
    : QObject(nullptr)
    , mImpl(new TimeServiceImpl)
{
    mImpl->context = context;
    mImpl->timeSyncUDPGB = new TimeSyncUDPGB;
    mImpl->thread = new QThread;
    mImpl->timeSyncUDPGB->moveToThread(mImpl->thread);
    mImpl->thread->start();

    connect(this, &TimeService::sg_start, mImpl->timeSyncUDPGB, &TimeSyncUDPGB::initSocket);
    connect(mImpl->timeSyncUDPGB, &TimeSyncUDPGB::sg_onReadyRead, [](const TimeData& timeData) { GlobalData::setTimeData(timeData); });
    emit sg_start();
}

TimeService::~TimeService()
{
    mImpl->thread->quit();
    mImpl->thread->wait();
    delete mImpl->thread;
    delete mImpl->timeSyncUDPGB;
    delete mImpl;
}
