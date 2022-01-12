#ifndef LSYDJYXBHLINKHANDLER_H
#define LSYDJYXBHLINKHANDLER_H

#include "BaseLinkHandler.h"

// 联试应答机有线闭环
class LSYDJYXBHLinkHandler : public BaseLinkHandler
{
    Q_OBJECT
public:
    explicit LSYDJYXBHLinkHandler(QObject* parent = nullptr);

    bool handle() override;

    LinkType getLinkType() override;

private:
    bool ckjdLink();

private:
    CKJDControlInfo m_ckjdInfo;
};

#endif  // LSYDJYXBHLINKHANDLER_H
