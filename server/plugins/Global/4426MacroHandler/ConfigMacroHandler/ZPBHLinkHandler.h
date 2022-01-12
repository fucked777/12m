#ifndef ZPBHLINKHANDLER_H
#define ZPBHLINKHANDLER_H

#include "BaseLinkHandler.h"

// 中频闭环
class ZPBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit ZPBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;

private:
    bool dsjdLink();

private:
    DeviceID m_dsjdDeviceID;
};

#endif  // ZPBHLINKHANDLER_H
