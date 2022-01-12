#ifndef LINKRESET_H
#define LINKRESET_H

#include "BaseLinkHandler.h"

// 任务方式
class LinkReset : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit LinkReset(QObject* parent = nullptr);

    bool handle() override;
    LinkType getLinkType() override;

};
#endif  // LINKRESET_H
