#ifndef KAGSRESOURCERELEASEHANDLER_H
#define KAGSRESOURCERELEASEHANDLER_H

#include "BaseResourceReleaseHandler.h"

class KaGSResourceReleaseHandler : public BaseResourceReleaseHandler
{
    Q_OBJECT

public:
    explicit KaGSResourceReleaseHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;
};

#endif  // KAGSRESOURCERELEASEHANDLER_H
