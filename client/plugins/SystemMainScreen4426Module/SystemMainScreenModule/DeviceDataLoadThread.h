#ifndef DeviceDataLoadThread_H
#define DeviceDataLoadThread_H

#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include <QObject>
#include <QThread>
#include <atomic>

struct DeviceStatusTypeInfo;
class DeviceDataLoadThread : public QThread
{
    Q_OBJECT
public:
    DeviceDataLoadThread();

    void stopRunning();


protected:
    void run() override;

private:
    std::atomic<bool> m_isRunning{ true };
};
#endif  // DeviceDataLoadThread_H
