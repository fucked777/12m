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
};

#endif  // STTCRESOURCERELEASEHANDLER_H
