#ifndef SPMNYYXBHLINKHANDLER_H
#define SPMNYYXBHLINKHANDLER_H

#include "BaseLinkHandler.h"

// 射频模拟源有线闭环
class SPMNYYXBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit SPMNYYXBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;
};

#endif  // SPMNYYXBHLINKHANDLER_H
