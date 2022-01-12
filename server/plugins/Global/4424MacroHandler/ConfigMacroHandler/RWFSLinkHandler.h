#ifndef RWFSLINKHANDLER_H
#define RWFSLINKHANDLER_H

#include "BaseLinkHandler.h"

// 任务方式
class RWFSLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit RWFSLinkHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;

private:
    // 处理测控任务方式
    bool handleCKRWFS();

    // 处理数传任务方式
    bool handleDTRWFS();
};
#endif  // RWFSLINKHANDLER_H
