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

    QThread thread;
    TimeSyncUDPGB timeSyncUDPGB;
};
TimeService::TimeService(cppmicroservices::BundleContext context)
    : QObject(nullptr)
    , mImpl(new TimeServiceImpl)
{
    mImpl->context = context;
    mImpl->timeSyncUDPGB.moveToThread(&(mImpl->thread));
    mImpl->thread.start();

    connect(this, &TimeService::sg_start, &(mImpl->timeSyncUDPGB), &TimeSyncUDPGB::initSocket);
    connect(&(mImpl->timeSyncUDPGB), &TimeSyncUDPGB::sg_onReadyRead, [](const TimeData& timeData) { GlobalData::setTimeData(timeData); });
    emit sg_start();
}

TimeService::~TimeService() { delete mImpl; }
