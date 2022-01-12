#include "ParamMacroDispatcher.h"

#include "KaGSParamMacroHandler.h"
#include "STTCParamMacroHandler.h"
#include "XGSParamMacroHandler.h"

ParamMacroDispatcher::ParamMacroDispatcher(QObject* parent)
    : QObject(parent)
    , mRunningFlag(nullptr)
{
    appendHandler(new STTCParamMacroHandler());
    appendHandler(new KaGSParamMacroHandler());
    appendHandler(new XGSParamMacroHandler());
}

ParamMacroDispatcher::~ParamMacroDispatcher()
{
    for (auto& item : mParamMacrohandleMap)
    {
        delete item;
    }
    mParamMacrohandleMap.clear();
}

void ParamMacroDispatcher::setRunningFlag(std::atomic<bool>* runningFlag) { mRunningFlag = runningFlag; }
bool ParamMacroDispatcher::isRunning()
{
    if (mRunningFlag == nullptr)
    {
        return true;
    }
    return mRunningFlag->load();
}
bool ParamMacroDispatcher::isExit()
{
    if (mRunningFlag == nullptr)
    {
        return false;
    }
    return !mRunningFlag->load();
}

void ParamMacroDispatcher::handle(const ManualMessage& msg)
{
    for (const auto& link : msg.linkLineMap)
    {
        if (!mRunningFlag)
        {
            return;
        }

        SystemWorkMode workMode = NotDefine;
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))  // 测控模式参数宏都使用同一个
        {
            workMode = SystemWorkMode::STTC;
        }
        else
        {
            workMode = link.workMode;
        }

        BaseParamMacroHandler* handler = mParamMacrohandleMap.value(workMode);
        if (handler == nullptr)
        {
            continue;
        }

        auto linkWorModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(link.workMode);
        emit signalInfoMsg(QString("%1模式参数宏开始执行").arg(linkWorModeDesc));

        handler->setManualMessage(msg);
        handler->setLinkLine(link);
        handler->setRunningFlag(mRunningFlag);
        if (handler->handle())
        {
            emit signalInfoMsg(QString("%1模式参数宏配置完成").arg(linkWorModeDesc));
        }
        else
        {
            emit signalErrorMsg(QString("%1模式参数宏配置错误").arg(linkWorModeDesc));
        }
    }
}

void ParamMacroDispatcher::appendHandler(BaseParamMacroHandler* paramMacroHandler)
{
    if (paramMacroHandler != nullptr)
    {
        connect(paramMacroHandler, &BaseParamMacroHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);

        mParamMacrohandleMap[paramMacroHandler->getSystemWorkMode()] = paramMacroHandler;
    }
}
