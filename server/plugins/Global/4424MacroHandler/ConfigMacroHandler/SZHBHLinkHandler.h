#ifndef SZHBHLINKHANDLER_H
#define SZHBHLINKHANDLER_H

#include "BaseLinkHandler.h"

// 数字化闭环
class SZHBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit SZHBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;
};

#endif  // SZHBHLINKHANDLER_H
