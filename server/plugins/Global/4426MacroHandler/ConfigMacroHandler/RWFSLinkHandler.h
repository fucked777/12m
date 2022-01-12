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
    bool handleCK();
    bool ckjdLink();
    bool ckjdLinkSingle();
    bool ckjdLinkDouble();

    bool handleDS();
    bool dsjdLink();
    bool handleGS();

    /* 当前低速的数量 */
    int dsModeNum() const;

private:
    CKJDControlInfo m_ckjdInfo;
    MasterSlaveDeviceIDInfo m_dsjdInfo;
    QSet<SystemWorkMode> m_workModeSet;
};
#endif  // RWFSLINKHANDLER_H
