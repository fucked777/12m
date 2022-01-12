#include "LinkReset.h"

#include "GlobalData.h"
#include "RedisHelper.h"
#include <QDebug>

LinkReset::LinkReset(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool LinkReset::handle()
{
    // 执行链路命令
    mManualMsg.manualType = ManualType::ConfigMacro;

    mManualMsg.linkLineMap.clear();

    LinkLine linkLine;
    linkLine.linkType = LinkType::LinkReset;
    linkLine.workMode = mManualMsg.linkMasterSlave == MasterSlave::Master?STTC:KaKuo2;
    linkLine.masterTargetNo = 1;
    mManualMsg.linkLineMap.insert(linkLine.workMode, linkLine);
    mLinkLine = linkLine;
    execLinkCommand();
    return true;
}

LinkType LinkReset::getLinkType() { return LinkType::Unknown; }
