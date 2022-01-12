#include "ConfigMacroDispatcher.h"

#include "GZJDHandler.h"
#include "LSYDJYXBHLinkHandler.h"
#include "PKXLBPQBHLinkHandler.h"
#include "RWFSLinkHandler.h"
#include "SPMNYWXBHLinkHandler.h"
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
    appendHandle(new SPMNYWXBHLinkHandler());
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
    auto result = BaseHandler::manualMessageTaskCheckOperation(msg);
    if (!result)
    {
        emit signalErrorMsg(result.msg());
        return;
    }
    auto linkType = result.value();
    for (const auto& link : msg.linkLineMap)
    {
        ExitCheckVoid();
        BaseLinkHandler* handler = mLinkhandleMap.value(link.linkType);
        if (handler == nullptr)
        {
            return;
        }

        auto linkTypeDesc = PlanRunHelper::getLinkTypeDesc(handler->getLinkType());
        auto linkWorModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(link.workMode);

        emit signalSpecificTipsMsg(QString("%1模式%2开始执行").arg(linkWorModeDesc, linkTypeDesc));

        handler->setManualMessage(msg);
        handler->setLinkLine(link);
        handler->setRunningFlag(mRunningFlag);

        if (handler->handle())
        {
            emit signalSpecificTipsMsg(QString("%1模式%2配置完成").arg(linkWorModeDesc, linkTypeDesc));
        }
        else
        {
            emit signalErrorMsg(QString("%1模式%2配置错误").arg(linkWorModeDesc, linkTypeDesc));
        }
        /* 命令连发 模式之间相互干扰 延时 */
        QThread::msleep(3000);
    }

    if (linkType == LinkType::RWFS && !(msg.linkLineMap.size() == 1 && msg.linkLineMap.contains(XDS)))
    {
        GZJDHandler gzjdHandler;
        gzjdHandler.setManualMessage(msg);
        gzjdHandler.setRunningFlag(mRunningFlag);
        connect(&gzjdHandler, &BaseHandler::signalSendToDevice, this, &ConfigMacroDispatcher::signalSendToDevice);
        connect(&gzjdHandler, &BaseHandler::signalInfoMsg, this, &ConfigMacroDispatcher::signalInfoMsg);
        connect(&gzjdHandler, &BaseHandler::signalWarningMsg, this, &ConfigMacroDispatcher::signalWarningMsg);
        connect(&gzjdHandler, &BaseHandler::signalErrorMsg, this, &ConfigMacroDispatcher::signalErrorMsg);
        connect(&gzjdHandler, &BaseHandler::signalSpecificTipsMsg, this, &ConfigMacroDispatcher::signalSpecificTipsMsg);

        gzjdHandler.handle();
    }

    emit signalSpecificTipsMsg(QString("配置宏配置完成"));
}

void ConfigMacroDispatcher::appendHandle(BaseLinkHandler* linkHandler)
{
    if (linkHandler != nullptr)
    {
        connect(linkHandler, &BaseHandler::signalSendToDevice, this, &ConfigMacroDispatcher::signalSendToDevice);
        connect(linkHandler, &BaseHandler::signalInfoMsg, this, &ConfigMacroDispatcher::signalInfoMsg);
        connect(linkHandler, &BaseHandler::signalWarningMsg, this, &ConfigMacroDispatcher::signalWarningMsg);
        connect(linkHandler, &BaseHandler::signalErrorMsg, this, &ConfigMacroDispatcher::signalErrorMsg);
        connect(linkHandler, &BaseHandler::signalSpecificTipsMsg, this, &ConfigMacroDispatcher::signalSpecificTipsMsg);

        mLinkhandleMap[linkHandler->getLinkType()] = linkHandler;
    }
}
