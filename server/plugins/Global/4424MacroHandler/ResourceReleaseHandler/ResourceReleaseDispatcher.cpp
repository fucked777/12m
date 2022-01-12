#include "ResourceReleaseDispatcher.h"
#include "KaGSResourceReleaseHandler.h"
#include "STTCResourceReleaseHandler.h"
#include "XGSResourceReleaseHandler.h"

ResourceReleaseDispatcher::ResourceReleaseDispatcher()
    : QObject()
{
    appendHandler(new STTCResourceReleaseHandler());
    appendHandler(new KaGSResourceReleaseHandler());
    appendHandler(new XGSResourceReleaseHandler());
}

ResourceReleaseDispatcher::~ResourceReleaseDispatcher()
{
    for (auto& item : mResourceReleaseHandlerMap)
    {
        delete item;
        item = nullptr;
    }
    mResourceReleaseHandlerMap.clear();
}

void ResourceReleaseDispatcher::handle(const ManualMessage& msg)
{
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
}

void ResourceReleaseDispatcher::appendHandler(BaseResourceReleaseHandler* resourceReleaseHandler)
{
    if (resourceReleaseHandler != nullptr)
    {
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalSendData, this, &ResourceReleaseDispatcher::signalSendData);

        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalSendToDevice, this, &ResourceReleaseDispatcher::signalSendToDevice);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalInfoMsg, this, &ResourceReleaseDispatcher::signalInfoMsg);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalWarningMsg, this, &ResourceReleaseDispatcher::signalWarningMsg);
        connect(resourceReleaseHandler, &BaseResourceReleaseHandler::signalErrorMsg, this, &ResourceReleaseDispatcher::signalErrorMsg);

        mResourceReleaseHandlerMap[resourceReleaseHandler->getSystemWorkMode()] = resourceReleaseHandler;
    }
}
