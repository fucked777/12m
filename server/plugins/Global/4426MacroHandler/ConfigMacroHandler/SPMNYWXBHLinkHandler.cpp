#include "SPMNYWXBHLinkHandler.h"

SPMNYWXBHLinkHandler::SPMNYWXBHLinkHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

bool SPMNYWXBHLinkHandler::handle() { return false; }

LinkType SPMNYWXBHLinkHandler::getLinkType() { return LinkType::SPMNYWXBH; }
