#include "ControlFlowHandler.h"

#include "ConfigMacroDispatcher.h"
#include "DeviceAlloter.h"
#include "OnekeyXLHandler.h"
#include "ParamMacroDispatcher.h"
#include "PlanRunMessageDefine.h"
#include "ResourceReleaseDispatcher.h"

#include "ConfigMacroMessageSerialize.h"

ControlFlowHandler::ControlFlowHandler(QObject* parent)
    : QObject(parent)
{
}
ControlFlowHandler::~ControlFlowHandler() {}
void ControlFlowHandler::exit() { mRunningFlag = false; }

OptionalNotValue ControlFlowHandler::allocConfigMacro(ManualMessage& msg, const QString& masterTaskCode)
{
    DeviceAlloter alloter;
    //    alloter.setMasterTaskCode(masterTaskCode);
    QString errorMsg;
    if (!alloter.allot(msg, errorMsg))
    {
        return OptionalNotValue(ErrorCode::InvalidData, errorMsg);
    }
    return OptionalNotValue();
}

void ControlFlowHandler::handle(const ManualMessage& msg, const QString& masterTaskCode)
{
    mRunningFlag = true;
    mManualMessage = msg;
    // 进行设备分配
    DeviceAlloter alloter;
    QString errorMsg;
    // TODO 818
    if (!alloter.allot(mManualMessage, errorMsg))
    {
        emit signalErrorMsg(errorMsg);
        return;
    }

    // QString json;
    // json << mManualMessage.configMacroData;
    // qDebug().noquote() << json;

    switch (mManualMessage.manualType)
    {
    case ManualType::ConfigMacro:  // 处理配置宏
    {
        if (mConfigMacroDispatcher == nullptr)
        {
            mConfigMacroDispatcher.reset(new ConfigMacroDispatcher);
            mConfigMacroDispatcher->setRunningFlag(&mRunningFlag);
        }

        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
        mConfigMacroDispatcher->handle(mManualMessage);
    }
    break;
    case ManualType::ParamMacro:  // 处理参数宏
    {
        if (mParamMacroDispatcher == nullptr)
        {
            mParamMacroDispatcher.reset(new ParamMacroDispatcher);
            mParamMacroDispatcher->setRunningFlag(&mRunningFlag);
        }

        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
        mParamMacroDispatcher->handle(mManualMessage);
    }
    break;
    case ManualType::ConfigMacroAndParamMacro:  // 处理配置宏和参数宏
    {
        if (mConfigMacroDispatcher == nullptr)
        {
            mConfigMacroDispatcher.reset(new ConfigMacroDispatcher);
            mConfigMacroDispatcher->setRunningFlag(&mRunningFlag);
        }
        if (mParamMacroDispatcher == nullptr)
        {
            mParamMacroDispatcher.reset(new ParamMacroDispatcher);
            mParamMacroDispatcher->setRunningFlag(&mRunningFlag);
        }

        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
        mConfigMacroDispatcher->handle(mManualMessage);

        if (!mRunningFlag)
        {
            return;
        }
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
        mParamMacroDispatcher->handle(mManualMessage);
    }
    break;
    case ManualType::ResourceRelease:  // 处理资源释放
    {
        if (mResourceReleaseDispatcher == nullptr)
        {
            mResourceReleaseDispatcher.reset(new ResourceReleaseDispatcher);
        }
        connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
        mResourceReleaseDispatcher->handle(mManualMessage);
    }
    break;
    case ManualType::OnekeyXL:  // 处理一键校零
    {
        if (mOnekeyXLHandler == nullptr)
        {
            mOnekeyXLHandler.reset(new OnekeyXLHandler);
        }
        connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
        connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
        connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
        connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);

        mOnekeyXLHandler->setManualMessage(mManualMessage);
        for (const auto& link : msg.linkLineMap)
        {
            mOnekeyXLHandler->setLinkLine(link);
            mOnekeyXLHandler->handle();
        }
    }
    break;
    default: break;
    }
}

ManualMessage ControlFlowHandler::getNewManualMsg() const { return mManualMessage; }

ConfigMacroData ControlFlowHandler::getNewConfigMacroData() const { return mManualMessage.configMacroData; }
