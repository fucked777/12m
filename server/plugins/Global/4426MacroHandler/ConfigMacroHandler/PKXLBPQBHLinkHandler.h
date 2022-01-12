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

private:
    bool ckjdLink();
    bool ckjdLinkSingle();
    bool ckjdLinkDouble();

private:
    CKJDControlInfo m_ckjdInfo;
    QSet<SystemWorkMode> m_workModeSet;
};

#endif  // PKXLBPQBHLINKHANDLER_H
