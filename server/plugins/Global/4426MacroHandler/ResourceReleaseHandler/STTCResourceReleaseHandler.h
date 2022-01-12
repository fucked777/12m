#ifndef STTCRESOURCERELEASEHANDLER_H
#define STTCRESOURCERELEASEHANDLER_H

#include "BaseResourceReleaseHandler.h"

class STTCResourceReleaseHandler : public BaseResourceReleaseHandler
{
    Q_OBJECT
public:
    explicit STTCResourceReleaseHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    bool sggf();
    bool kagf();
    bool sttc(const DeviceID& deviceID);
    bool skuo2(const DeviceID& deviceID);
    bool kakuo2(const DeviceID& deviceID);
    bool skt(const DeviceID& deviceID);
};

#endif  // STTCRESOURCERELEASEHANDLER_H
