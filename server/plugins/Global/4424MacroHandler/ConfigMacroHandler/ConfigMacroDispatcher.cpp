#include "ConfigMacroDispatcher.h"

#include "LSYDJYXBHLinkHandler.h"
#include "PKXLBPQBHLinkHandler.h"
#include "RWFSLinkHandler.h"
#include "SPMNYYXBHLinkHandler.h"
#include "SZHBHLinkHandler.h"
#include "ZPBHLinkHandler.h"

ConfigMacroDispatcher::ConfigMacroDispatcher(QObject* parent)
    : QObject(parent)
    , mRunningFlag(nullptr)
{
    appendHandle(new RWFSLinkHandler());
    appendHandle(new LSYDJYXBHLinkHandler());
    appendHandle(new PKXLBPQBHLinkHandler());
    appendHandle(new SPMNYYXBHLinkHandler());
    appendHandle(new ZPBHLinkHandler());
    appendHandle(new SZHBHLinkHandler());
}

ConfigMacroDispatcher::~ConfigMacroDispatcher()
{
    for (auto& item : mLinkhandleMap)
    {
        delete item;
    }
    mLinkhandleMap.clear();
}

void ConfigMacroDispatcher::setRunningFlag(std::atomic<bool>* runningFlag) { mRunningFlag = runningFlag; }
bool ConfigMacroDispatcher::isRunning()
{
    if (mRunningFlag == nullptr)
    {
        return true;
    }
    return mRunningFlag->load();
}
bool ConfigMacroDispatcher::isExit()
{
    if (mRunningFlag == nullptr)
    {
        return false;
    }
    return !mRunningFlag->load();
}

void ConfigMacroDispatcher::handle(const ManualMessage& msg)
{
    for (const auto& link : msg.linkLineMap)
    {
        if (!mRunningFlag)
        {
            return;
        }

        BaseLinkHandler* handler = mLinkhandleMap.value(link.linkType);
        if (handler == nullptr)
        {
            return;
        }

        auto linkTypeDesc = PlanRunHelper::getLinkTypeDesc(handler->getLinkType());
        auto linkWorModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(link.workMode);

        emit signalInfoMsg(QString("%1模式%2开始执行").arg(linkWorModeDesc, linkTypeDesc));

        handler->setManualMessage(msg);
        handler->setLinkLine(link);
        handler->setRunningFlag(mRunningFlag);

        if (handler->handle())
        {
            emit signalInfoMsg(QString("%1模式%2配置完成").arg(linkWorModeDesc, linkTypeDesc));
        }
        else
        {
            emit signalErrorMsg(QString("%1模式%2配置错误").arg(linkWorModeDesc, linkTypeDesc));
        }
    }
}

void ConfigMacroDispatcher::appendHandle(BaseLinkHandler* linkHandler)
{
    if (linkHandler != nullptr)
    {
        connect(linkHandler, &BaseHandler::signalSendToDevice, this, &ConfigMacroDispatcher::signalSendToDevice);
        connect(linkHandler, &BaseHandler::signalInfoMsg, this, &ConfigMacroDispatcher::signalInfoMsg);
        connect(linkHandler, &BaseHandler::signalWarningMsg, this, &ConfigMacroDispatcher::signalWarningMsg);
        connect(linkHandler, &BaseHandler::signalErrorMsg, this, &ConfigMacroDispatcher::signalErrorMsg);

        mLinkhandleMap[linkHandler->getLinkType()] = linkHandler;
    }
}
