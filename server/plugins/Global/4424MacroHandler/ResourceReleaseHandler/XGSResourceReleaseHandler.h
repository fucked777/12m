#ifndef XGSRESOURCERELEASEHANDLER_H
#define XGSRESOURCERELEASEHANDLER_H

#include "BaseResourceReleaseHandler.h"

class XGSResourceReleaseHandler : public BaseResourceReleaseHandler
{
    Q_OBJECT

public:
    explicit XGSResourceReleaseHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;
};

#endif  // XGSRESOURCERELEASEHANDLER_H
