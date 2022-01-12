#include "ControlFlowHandler.h"

#include "ConfigMacroDispatcher.h"
#include "ConfigMacroMessageSerialize.h"
#include "DeviceAlloter.h"
#include "EquipmentCombinationHelper.h"
#include "MessagePublish.h"
#include "OnekeyXLHandler.h"
#include "OnekeyXXHandler.h"
#include "ParamMacroDispatcher.h"
#include "PlanRunMessageDefine.h"
#include "ResourceReleaseDispatcher.h"
#include "SatelliteManagementSerialize.h"
#include "StationResManagementSerialize.h"
#include "TaskPlanMessageSerialize.h"

ControlFlowHandler::ControlFlowHandler(QObject* parent)
    : QObject(parent)
{
}
ControlFlowHandler::~ControlFlowHandler() {}
OptionalNotValue ControlFlowHandler::allocConfigMacro(ManualMessage& msg, bool isCheck)
{
    // 数据检查
    if (isCheck)
    {
        auto checkResult = checkAndFillManualMessage(msg);
        if (!checkResult)
        {
            return checkResult;
        }
    }

    DeviceAlloter alloter;
    QString errorMsg;
    if (!alloter.allot(msg, errorMsg))
    {
        return OptionalNotValue(ErrorCode::InvalidData, errorMsg);
    }
    return OptionalNotValue();
}

// 检查和修正主根任务代号
OptionalNotValue ControlFlowHandler::checkAndFillManualMessage(ManualMessage& manualMessage)
{
    if (manualMessage.masterTaskCode.isEmpty())
    {
        auto msg = QString("当前主跟任务代号为空,流程无法继续");
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }
    bool existTaskCode = false;
    for (auto& link : manualMessage.linkLineMap)
    {
        for (auto& targetInfo : link.targetMap)
        {
            if (targetInfo.taskCode == manualMessage.masterTaskCode)
            {
                existTaskCode = true;
                break;
            }
        }
    }
    if (!existTaskCode)
    {
        auto msg = QString("当前流程参数中未找到主跟代号为%1的点频数据").arg(manualMessage.masterTaskCode);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }
    SatelliteManagementData satelliteManagementData;
    if (!GlobalData::getSatelliteManagementData(manualMessage.masterTaskCode, satelliteManagementData))
    {
        auto msg = QString("未查找到主跟代号为%1的卫星信息").arg(manualMessage.masterTaskCode);
        return OptionalNotValue(ErrorCode::NotFound, msg);
    }
    manualMessage.masterTaskBz = satelliteManagementData.m_satelliteIdentification;

    auto staticUACResult = BaseHandler::getStationCKUAC();
    if (!staticUACResult)
    {
        return OptionalNotValue(staticUACResult.errorCode(), staticUACResult.msg());
    }
    manualMessage.txUACAddr = staticUACResult.value();

    for (auto& link : manualMessage.linkLineMap)
    {
        if (link.targetMap.isEmpty())
        {
            auto msg = QString("当前配置错误目标信息为空,流程无法继续");
            return OptionalNotValue(ErrorCode::InvalidArgument, msg);
        }

        // 没找到就默认选择第一个
        if (!link.targetMap.contains(link.masterTargetNo))
        {
            link.masterTargetNo = link.targetMap.firstKey();
        }
    }

    return OptionalNotValue();
}

void ControlFlowHandler::getDevNumber()
{
    // 如果有多种模式，优先判断是否有测控工作模式，没有再使用数传工作模式
    SystemWorkMode workMode = NotDefine;
    for (auto linkWokMode : mManualMessage.linkLineMap.keys())
    {
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(linkWokMode))
        {
            workMode = linkWokMode;
            break;
        }
    }

    quint64 equipComb = 0;

    // 有测控模式
    if (workMode != NotDefine)
    {
        // 获取测控基带
        DeviceID ckjdDeviceID;
        mManualMessage.configMacroData.getCKJDDeviceID(workMode, ckjdDeviceID, 1);

        // 获取跟踪基带
        DeviceID gzjdDeviceID;
        mManualMessage.configMacroData.getGZJDDeviceID(workMode, gzjdDeviceID);

        if (!EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMessage.configMacroData, ckjdDeviceID, gzjdDeviceID, equipComb))
        {
            emit signalErrorMsg("获取当前设备组合号失败");
            return;
        }
    }
    else
    {
        // 获取数传工作模式
        workMode = mManualMessage.linkLineMap.firstKey();

        // 获取跟踪基带
        DeviceID gzjdDeviceID;
        mManualMessage.configMacroData.getGZJDDeviceID(workMode, gzjdDeviceID);

        if (!EquipmentCombinationHelper::getDTDevNumber(workMode, mManualMessage.configMacroData, gzjdDeviceID, equipComb))
        {
            emit signalErrorMsg("获取当前设备组合号失败");
            return;
        }
    }

    mManualMessage.devNumber = equipComb;
}
void ControlFlowHandler::setPlanInfo(const TaskPlanData& taskPlanData) { mTaskPlanData = taskPlanData; }
void ControlFlowHandler::handle(const ManualMessage& msg, bool isCheck)
{
    mManualMessage = msg;

    /* 资源释放是特殊的 */
    if (mManualMessage.manualType != ManualType::ResourceRelease)
    {
        // 数据检查
        if (isCheck)
        {
            auto checkResult = checkAndFillManualMessage(mManualMessage);
            if (!checkResult)
            {
                emit signalErrorMsg(checkResult.msg());
                return;
            }
        }

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

        // 获取设备组合号
        getDevNumber();

        // 设置到全局中 不能写在这里 校零和资源释放不算任务
        // GlobalData::setTaskRunningInfo(msg);
    }

    switch (mManualMessage.manualType)
    {
    case ManualType::ConfigMacro:  // 处理配置宏
    {
        if (mConfigMacroDispatcher == nullptr)
        {
            mConfigMacroDispatcher.reset(new ConfigMacroDispatcher);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }

        mConfigMacroDispatcher->setRunningFlag(mRunningFlag);
        mConfigMacroDispatcher->handle(mManualMessage);
        break;
    }
    case ManualType::ParamMacro:  // 处理参数宏
    {
        if (mParamMacroDispatcher == nullptr)
        {
            mParamMacroDispatcher.reset(new ParamMacroDispatcher);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }

        mParamMacroDispatcher->setCCZFPlan(mTaskPlanData);
        mParamMacroDispatcher->setRunningFlag(mRunningFlag);
        mParamMacroDispatcher->handle(mManualMessage);
        break;
    }
    case ManualType::ConfigMacroAndParamMacro:  // 处理配置宏和参数宏
    {
        if (mConfigMacroDispatcher == nullptr)
        {
            mConfigMacroDispatcher.reset(new ConfigMacroDispatcher);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mConfigMacroDispatcher.data(), &ConfigMacroDispatcher::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }
        if (mParamMacroDispatcher == nullptr)
        {
            mParamMacroDispatcher.reset(new ParamMacroDispatcher);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mParamMacroDispatcher.data(), &ParamMacroDispatcher::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }

        mConfigMacroDispatcher->setRunningFlag(mRunningFlag);
        mConfigMacroDispatcher->handle(mManualMessage);

        ExitCheckVoid();
        mParamMacroDispatcher->setCCZFPlan(mTaskPlanData);
        mParamMacroDispatcher->setRunningFlag(mRunningFlag);
        mParamMacroDispatcher->handle(mManualMessage);
        break;
    }
    case ManualType::ResourceRelease:  // 处理资源释放
    {
        if (mResourceReleaseDispatcher == nullptr)
        {
            mResourceReleaseDispatcher.reset(new ResourceReleaseDispatcher);
            connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mResourceReleaseDispatcher.data(), &ResourceReleaseDispatcher::signalSpecificTipsMsg, this,
                    &ControlFlowHandler::signalSpecificTipsMsg);
        }

        /*
         * 资源释放清空任务运行信息
         * 在自动化运行的时候是先下宏此时设置了任务开始信息
         * 然后在预处理结束之前要资源释放此时会清空任务信息
         * 这里做一个处理 使用ControlFlowHandler的会清空 单独使用ResourceReleaseDispatcher不会清空
         *
         * 这里说明一下单独使用这里的每一个类都需要准备好mManualMessage
         * 自动化任务是保存了这个的信息的所以单独拿出来用也没关系
         */
        GlobalData::cleanTaskRunningInfo();
        mResourceReleaseDispatcher->setRunningFlag(mRunningFlag);
        mResourceReleaseDispatcher->handle(mManualMessage);
        break;
    }
    case ManualType::OnekeyXL:  // 处理一键校零
    {
        if (mOnekeyXLHandler == nullptr)
        {
            mOnekeyXLHandler.reset(new OnekeyXLHandler);
            connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mOnekeyXLHandler.data(), &OnekeyXLHandler::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }

        mOnekeyXLHandler->setRunningFlag(mRunningFlag);
        mOnekeyXLHandler->setManualMessage(mManualMessage);

        if (mManualMessage.xlMode == SystemWorkMode::NotDefine)
        {
            for (const auto& link : msg.linkLineMap)
            {
                mOnekeyXLHandler->setLinkLine(link);
                mOnekeyXLHandler->handle();
            }
        }
        else
        {
            mOnekeyXLHandler->setLinkLine(msg.linkLineMap.value(mManualMessage.xlMode));
            mOnekeyXLHandler->handle();
        }

        break;
    }
    case ManualType::OnekeyXX:  // 处理一键校相
    {
        if (mOnekeyXXHandler == nullptr)
        {
            mOnekeyXXHandler.reset(new OnekeyXXHandler);
            connect(mOnekeyXXHandler.data(), &OnekeyXLHandler::signalSendToDevice, this, &ControlFlowHandler::signalSendToDevice);
            connect(mOnekeyXXHandler.data(), &OnekeyXLHandler::signalInfoMsg, this, &ControlFlowHandler::signalInfoMsg);
            connect(mOnekeyXXHandler.data(), &OnekeyXLHandler::signalWarningMsg, this, &ControlFlowHandler::signalWarningMsg);
            connect(mOnekeyXXHandler.data(), &OnekeyXLHandler::signalErrorMsg, this, &ControlFlowHandler::signalErrorMsg);
            connect(mOnekeyXXHandler.data(), &OnekeyXLHandler::signalSpecificTipsMsg, this, &ControlFlowHandler::signalSpecificTipsMsg);
        }

        mOnekeyXXHandler->setRunningFlag(mRunningFlag);
        mOnekeyXXHandler->setManualMessage(mManualMessage);
        mOnekeyXXHandler->handle();
        break;
    }
    default: break;
    }
}
void ControlFlowHandler::setRunningFlag(std::atomic<bool>* runningFlag) { mRunningFlag = runningFlag; }
bool ControlFlowHandler::isRunning()
{
    if (mRunningFlag == nullptr)
    {
        return true;
    }
    return mRunningFlag->load();
}
bool ControlFlowHandler::isExit()
{
    if (mRunningFlag == nullptr)
    {
        return false;
    }
    return !mRunningFlag->load();
}

ManualMessage ControlFlowHandler::getNewManualMsg() const { return mManualMessage; }

ConfigMacroData ControlFlowHandler::getNewConfigMacroData() const { return mManualMessage.configMacroData; }
