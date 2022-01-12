#ifndef DSRESOURCERELEASEHANDLER_H
#define DSRESOURCERELEASEHANDLER_H

#include "BaseResourceReleaseHandler.h"

class DSResourceReleaseHandler : public BaseResourceReleaseHandler
{
    Q_OBJECT
public:
    explicit DSResourceReleaseHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    void releaseItem(const DeviceID& deviceID);
};

#endif  // DSRESOURCERELEASEHANDLER_H
