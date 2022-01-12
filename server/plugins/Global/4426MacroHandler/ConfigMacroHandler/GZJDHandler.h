#ifndef GZJDHANDLER_H
#define GZJDAHNDLER_H

#include "BaseLinkHandler.h"

// 跟踪基带链路处理
// 此类为内部使用类 单独使用一定会出错
class GZJDHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit GZJDHandler(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;
};
#endif  // GZJDAHNDLER_H
