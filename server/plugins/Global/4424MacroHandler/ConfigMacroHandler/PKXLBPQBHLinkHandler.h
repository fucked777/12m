#ifndef PKXLBPQBHLINKHANDLER_H
#define PKXLBPQBHLINKHANDLER_H

#include "BaseLinkHandler.h"

// 偏馈校零变频器闭环
class PKXLBPQBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit PKXLBPQBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;
};

#endif  // PKXLBPQBHLINKHANDLER_H
