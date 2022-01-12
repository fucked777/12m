#include "ResourceReleaseDispatcher.h"
#include "DSResourceReleaseHandler.h"
#include "KaGSResourceReleaseHandler.h"
#include "LinkReset.h"
#include "OtherResourceReleaseHandler.h"
#include "STTCResourceReleaseHandler.h"

ResourceReleaseDispatcher::ResourceReleaseDispatcher()
    : QObject()
{
    appendHandler(new STTCResourceReleaseHandler());
    appendHandler(new KaGSResourceReleaseHandler());
    appendHandler(new DSResourceReleaseHandler());
    appendHandler(new OtherResourceReleaseHandler());

    mRestLink = new LinkReset();
    connect(mRestLink, &BaseResourceReleaseHandler::signalSendToDevice, this, &ResourceReleaseDispatcher::signalSendToDevice);
    connect(mRestLink, &BaseResourceReleaseHandler::signalInfoMsg, this, &ResourceReleaseDispatcher::signalInfoMsg);
    connect(mRestLink, &BaseResourceReleaseHandler::signalWarningMsg, this, &ResourceReleaseDispatcher::signalWarningMsg);
    connect(mRestLink, &BaseResourceReleaseHandler::signalErrorMsg, this, &ResourceReleaseDispatcher::signalErrorMsg);
}
ResourceReleaseDispatcher::~ResourceReleaseDispatcher()
{
    for (auto& item : mResourceReleaseHandlerMap)
    {
        delete item;
    }
    mResourceReleaseHandlerMap.clear();
    delete mRestLink;
    mRestLink = nullptr;
}
void ResourceReleaseDispatcher::handle(const ManualMessage& msg)
{
    emit signalSpecificTipsMsg(QString("资源释放开始"));
#if 1
    /* 20211123 更改策略为全部释放 不用管当前的模式 */
    for (auto& handler : mResourceReleaseHandlerMap)
    {
        if (handler == nullptr)
        {
            continue;
        }
        if (isExit())
        {
            return;
        }
        handler->setRunningFlag(mRunningFlag);
        handler->setManualMessage(msg);
        handler->handle();
    }

    //mRestLink->setManualMessage(msg);
    //mRestLink->setRunningFlag(mRunningFlag);
    //mRestLink->handle();
#else
    for (const auto& link : msg.linkLineMap)
    {
        SystemWorkMode workMode = NotDefine;
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))  // 测控模式参数宏都使用同一个
        {
            workMode = SystemWorkMode::STTC;
        }
        else
        {
            workMode = link.workMode;
        }

        BaseResourceReleaseHandler* handler = mResourceReleaseHandlerMap.value(workMode);
        if (handler == nullptr)
        {
            continue;
        }
        handler->setManualMessage(msg);
        handler->setLinkLine(link);
        handler->handle();
    }
#endif

    emit signalSpecificTipsMsg(QString("资源释放完毕"));
}
void ResourceReleaseDispatcher::setRunningFlag(std::atomic<bool>* runningFlag) { mRunningFlag = runningFlag; }
bool ResourceReleaseDispatcher::isRunning()
{
    if (mRunningFlag == nullptr)
    {
        return true;
    }
    return mRunningFlag->load();
}
bool ResourceReleaseDispatcher::isExit()
{
    if (mRunningFlag == nullptr)
    {
        return false;
    }
    return !mRunningFlag->load();
}
void ResourceReleaseDispatcher::appendHandler(BaseResourceReleaseHandler* resourceReleaseHandler)
{
    if (resourceReleaseHandler != nullptr)
    {
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalSendToDevice, this, &ResourceReleaseDispatcher::signalSendToDevice);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalInfoMsg, this, &ResourceReleaseDispatcher::signalInfoMsg);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalWarningMsg, this, &ResourceReleaseDispatcher::signalWarningMsg);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalErrorMsg, this, &ResourceReleaseDispatcher::signalErrorMsg);

        mResourceReleaseHandlerMap[resourceReleaseHandler->getSystemWorkMode()] = resourceReleaseHandler;
    }
}
