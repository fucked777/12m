#ifndef RFWIRELESSMODELINKHANDLER_H
#define RFWIRELESSMODELINKHANDLER_H

#include "BaseLinkHandler.h"

// 射频模拟源无线闭环
class SPMNYWXBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit SPMNYWXBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;
};

#endif  // RFWIRELESSMODELINKHANDLER_H
