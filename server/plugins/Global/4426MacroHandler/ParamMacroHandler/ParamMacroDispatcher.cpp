#include "ParamMacroDispatcher.h"

#include "ACUParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "CCZFParamMacroHandler.h"
#include "DSParamMacroHandler.h"
#include "DTETaskHandler.h"
#include "KaGSParamMacroHandler.h"
#include "STTCParamMacroHandler.h"
#include "TrkBBEParamMacroHandler.h"
#include "UpLinkHandler.h"

class ParamMacroDispatcherImpl
{
public:
    QMap<SystemWorkMode, BaseParamMacroHandler*> paramMacrohandleMap;
    BaseParamMacroHandler* trkBBEHandler{ nullptr };
    /* 20210926 wp?? 退出的标志 */
    std::atomic<bool>* runningFlag{ nullptr };
    TaskPlanData taskPlanData;
};

ParamMacroDispatcher::ParamMacroDispatcher(QObject* parent)
    : QObject(parent)
    , m_impl(new ParamMacroDispatcherImpl)
{
    m_impl->trkBBEHandler = new TrkBBEParamMacroHandler;
    //跟踪基带参数宏，为了不影响其他模块，待验证完
    connect(m_impl->trkBBEHandler, &BaseParamMacroHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
    connect(m_impl->trkBBEHandler, &BaseParamMacroHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
    connect(m_impl->trkBBEHandler, &BaseParamMacroHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
    connect(m_impl->trkBBEHandler, &BaseParamMacroHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
    connect(m_impl->trkBBEHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

    appendHandler(new STTCParamMacroHandler());
    appendHandler(new KaGSParamMacroHandler());
    appendHandler(new DSParamMacroHandler());
}

ParamMacroDispatcher::~ParamMacroDispatcher()
{
    for (auto& item : m_impl->paramMacrohandleMap)
    {
        delete item;
    }
    m_impl->paramMacrohandleMap.clear();
    delete m_impl->trkBBEHandler;
    m_impl->trkBBEHandler = nullptr;
}

void ParamMacroDispatcher::setRunningFlag(std::atomic<bool>* runningFlag) { m_impl->runningFlag = runningFlag; }
bool ParamMacroDispatcher::isRunning()
{
    if (m_impl->runningFlag == nullptr)
    {
        return true;
    }
    return m_impl->runningFlag->load();
}
bool ParamMacroDispatcher::isExit()
{
    if (m_impl->runningFlag == nullptr)
    {
        return false;
    }
    return !m_impl->runningFlag->load();
}
void ParamMacroDispatcher::setCCZFPlan(const TaskPlanData& taskPlanData) { m_impl->taskPlanData = taskPlanData; }
void ParamMacroDispatcher::handle(const ManualMessage& msg)
{
    auto result = BaseHandler::manualMessageTaskCheckOperation(msg);
    if (!result)
    {
        emit signalErrorMsg(result.msg());
        return;
    }
    auto linkType = result.value();
    /* 旋向检查 */
    polarizationCheck(msg);

    /* 20211130 提前下 */
    // 控制ACU
    auto acuParam = (msg.linkLineMap.size() == 1 && msg.linkLineMap.contains(XDS)) ? false : msg.acuParam;
    if (acuParam && linkType == LinkType::RWFS)
    {
        ACUParamMacroHandler acuParamMacroHandler;
        acuParamMacroHandler.setManualMessage(msg);
        acuParamMacroHandler.setRunningFlag(m_impl->runningFlag);
        connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&acuParamMacroHandler, &ACUParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        acuParamMacroHandler.handle();
    }

    if (linkType == LinkType::RWFS)
    {
        /* 存储转发 */
        CCZFParamMacroHandler cczfParamMacroHandler;
        cczfParamMacroHandler.setManualMessage(msg);
        cczfParamMacroHandler.setRunningFlag(m_impl->runningFlag);
        cczfParamMacroHandler.setCCZFPlan(m_impl->taskPlanData);

        connect(&cczfParamMacroHandler, &DTETaskHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&cczfParamMacroHandler, &DTETaskHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&cczfParamMacroHandler, &DTETaskHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&cczfParamMacroHandler, &DTETaskHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&cczfParamMacroHandler, &DTETaskHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        cczfParamMacroHandler.handle();

        /* DTE */
        DTETaskHandler dteTaskHandler;
        if (!msg.ltDTETask)
        {
            dteTaskHandler.setDTETaskTime(msg.dteTraceStartDateTime, msg.dteTaskEndDateTime);
        }
        dteTaskHandler.setManualMessage(msg);
        dteTaskHandler.setRunningFlag(m_impl->runningFlag);
        connect(&dteTaskHandler, &DTETaskHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&dteTaskHandler, &DTETaskHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&dteTaskHandler, &DTETaskHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&dteTaskHandler, &DTETaskHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&dteTaskHandler, &DTETaskHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);
        dteTaskHandler.handle();
    }


    for (const auto& link : msg.linkLineMap)
    {
        ExitCheckVoid();
        SystemWorkMode workMode = NotDefine;
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))  // 测控模式参数宏都使用同一个
        {
            workMode = SystemWorkMode::STTC;
        }
        else
        {
            workMode = link.workMode;
        }

        /* 单X低速直接使用 Ka低速的 */
        if (msg.linkLineMap.size() == 1 && link.workMode == XDS)
        {
            workMode = KaDS;
        }

        BaseParamMacroHandler* handler = m_impl->paramMacrohandleMap.value(workMode);
        if (handler == nullptr)
        {
            continue;
        }

        auto linkWorModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(link.workMode);
        emit signalSpecificTipsMsg(QString("%1模式参数宏开始执行").arg(linkWorModeDesc));

        handler->setManualMessage(msg);
        handler->setLinkLine(link);
        handler->setRunningFlag(m_impl->runningFlag);

        if (handler->handle())
        {
            emit signalSpecificTipsMsg(QString("%1模式参数宏配置完成").arg(linkWorModeDesc));
        }
        else
        {
            emit signalErrorMsg(QString("%1模式参数宏配置错误").arg(linkWorModeDesc));
        }
    }

    //跟踪基带参数宏
    if (!(msg.linkLineMap.size() == 1 && msg.linkLineMap.contains(XDS)) && linkType == LinkType::RWFS)
    {
        m_impl->trkBBEHandler->setManualMessage(msg);
        m_impl->trkBBEHandler->setLinkLine(msg.linkLineMap.first());
        m_impl->trkBBEHandler->setRunningFlag(m_impl->runningFlag);
        emit signalSpecificTipsMsg(QString("跟踪基带参数宏开始执行"));
        if (m_impl->trkBBEHandler->handle())
        {
            emit signalSpecificTipsMsg(QString("跟踪基带参数宏配置完成"));
        }
        else
        {
            emit signalErrorMsg(QString("跟踪基带参数宏配置错误"));
        }
    }

    // 特殊处理
    // 当任务环路下完参数宏之后 要关闭所有的上行链路输出
    // 当射频模拟源闭环下完参数宏之后 要关闭所有的功放 开启上行输出
    // 当联试应答机闭环下完参数宏之后 要关闭所有的功放开启上行输出 如果存在Ka扩2要开启上行输出去负载
    // 当中频闭环数字化闭环 下完宏之后要关闭所有功放
    // 当校零环路 下完宏之后要开启上行输出去天线 校零可以多个模式挨着校零这里就不启用了直接全部禁用 需要校那个一键校零去做这个事儿

    // 额...20211026 这里认为我们的监控不能自动的同时下多种链路类型
    emit signalSpecificTipsMsg(QString("上行输出开始配置"));
    if (linkType == LinkType::RWFS ||  //
        linkType == LinkType::PKXLBPQBH)
    {
        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableHpa(EnableMode::Disable);  // 启用 禁用 忽略
        upLinkHandler.setEnableJD(EnableMode::Ignore);    // 启用 禁用 忽略
        upLinkHandler.setEnableQD(EnableMode::Enable);    // 启用 禁用 忽略
        upLinkHandler.enableControl(UpLinkDeviceControl::All);
        upLinkHandler.setManualMessage(msg);
        upLinkHandler.setRunningFlag(m_impl->runningFlag);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&upLinkHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        upLinkHandler.handle();
    }
    else if (linkType == LinkType::ZPBH ||  //
             linkType == LinkType::SZHBH)
    {
        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableHpa(EnableMode::Ignore);  // 启用 禁用 忽略
        upLinkHandler.setEnableJD(EnableMode::Enable);   // 启用 禁用 忽略
        upLinkHandler.setEnableQD(EnableMode::Enable);   // 启用 禁用 忽略
        upLinkHandler.enableControl(UpLinkDeviceControl::All);
        upLinkHandler.setIgnoreOtherDevice(true);
        upLinkHandler.setIgnoreXLDevice(true);
        upLinkHandler.setManualMessage(msg);
        upLinkHandler.setRunningFlag(m_impl->runningFlag);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&upLinkHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        upLinkHandler.handle();
    }
    else if (linkType == LinkType::SPMNYYXBH)
    {
        // 射频模拟源Ka低速 Ka测控 Ka高速不能同时 占用测试高频箱 前端测试开关网络
        // 再指定开启基带前端信号
        auto& link = msg.linkLineMap.first();

        UpLinkHandler upLinkHandler;
        upLinkHandler.setRunningFlag(m_impl->runningFlag);
        upLinkHandler.setEnableHpa(EnableMode::Ignore);  // 启用 禁用 忽略
        upLinkHandler.setEnableJD(EnableMode::Enable);   // 启用 禁用 忽略
        upLinkHandler.setEnableQD(EnableMode::Enable);   // 启用 禁用 忽略
        upLinkHandler.setIgnoreOtherDevice(true);
        upLinkHandler.setIgnoreXLDevice(true);

        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::CK);
        }
        else  // if (link.workMode == KaDS)
        {
            upLinkHandler.enableControl(UpLinkDeviceControl::SC);
        }

        upLinkHandler.setManualMessage(msg);
        upLinkHandler.setLinkLine(link);
        upLinkHandler.setQDSendSource(CKQDSendSource::JD);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&upLinkHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        upLinkHandler.handle();
    }
    else if (linkType == LinkType::LSYDJYXBH)
    {
        // 联试应答机同时只能有一种模式存在 联试应答机信道 联试应答机终端只有一个
        // 待确认Ka扩2 貌似不使用功放 只使用上变频器就行 wp??
        auto& link = msg.linkLineMap.first();

        UpLinkHandler upLinkHandler;
        upLinkHandler.setEnableJD(EnableMode::Enable);
        upLinkHandler.setEnableQD(EnableMode::Enable);
        upLinkHandler.setIgnoreOtherDevice(true);
        upLinkHandler.setIgnoreXLDevice(true);

        if (link.workMode == KaKuo2)
        {
            upLinkHandler.setEnableHpa(EnableMode::Ignore);
            upLinkHandler.enableControl(UpLinkDeviceControl::CK);
            // upLinkHandler.setRFOutputMode(RFOutputMode::FZ);
        }
        else if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))
        {
            upLinkHandler.setEnableHpa(EnableMode::Ignore);
            upLinkHandler.enableControl(UpLinkDeviceControl::CK);
        }
        else  // if (link.workMode == KaDS)
        {
            upLinkHandler.setEnableHpa(EnableMode::Ignore);
            upLinkHandler.enableControl(UpLinkDeviceControl::SC);
        }

        upLinkHandler.setRunningFlag(m_impl->runningFlag);
        upLinkHandler.setManualMessage(msg);
        upLinkHandler.setLinkLine(link);
        upLinkHandler.setQDSendSource(CKQDSendSource::JD);

        connect(&upLinkHandler, &UpLinkHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(&upLinkHandler, &UpLinkHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(&upLinkHandler, &UpLinkHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(&upLinkHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        upLinkHandler.handle();
    }
    emit signalSpecificTipsMsg(QString("上行输出配置完成"));

    //加这个是为了给客户端一个提示
    emit signalSpecificTipsMsg(QString("参数宏配置完成"));
}

void ParamMacroDispatcher::appendHandler(BaseParamMacroHandler* paramMacroHandler)
{
    if (paramMacroHandler != nullptr)
    {
        connect(paramMacroHandler, &BaseParamMacroHandler::signalSendToDevice, this, &ParamMacroDispatcher::signalSendToDevice);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalInfoMsg, this, &ParamMacroDispatcher::signalInfoMsg);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalWarningMsg, this, &ParamMacroDispatcher::signalWarningMsg);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalErrorMsg, this, &ParamMacroDispatcher::signalErrorMsg);
        connect(paramMacroHandler, &BaseParamMacroHandler::signalSpecificTipsMsg, this, &ParamMacroDispatcher::signalSpecificTipsMsg);

        m_impl->paramMacrohandleMap[paramMacroHandler->getSystemWorkMode()] = paramMacroHandler;
    }
}
void ParamMacroDispatcher::polarizationCheck(const ManualMessage& manualMessage)
{
    /* 获取卫星管理 */
    SatelliteManagementDataMap sateInfoMap;
    if (!GlobalData::getSatelliteManagementData(sateInfoMap))
    {
        emit signalErrorMsg("获取当前卫星管理数据错误");
        return;
    }

    ParamMacroDataMap paramMacroDataMap;
    if (!GlobalData::getAllParamMacroData(paramMacroDataMap))
    {
        emit signalErrorMsg("获取当前参数宏数据错误");
        return;
    }
    for (const auto& link : manualMessage.linkLineMap)
    {
        for (auto& item : link.targetMap)
        {
            auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(item.workMode);
            auto sateItem = sateInfoMap.value(item.taskCode);
            auto paramItem = paramMacroDataMap.value(item.taskCode);
            if (sateItem.m_workModeParamMap.isEmpty())
            {
                auto msg = QString("未查找的卫星代号为%1的卫星信息").arg(item.taskCode);
                emit signalErrorMsg(msg);
                continue;
            }
            if (paramItem.modeParamMap.isEmpty())
            {
                auto msg = QString("未查找的卫星代号为%1的卫星参数宏").arg(item.taskCode);
                emit signalErrorMsg(msg);
                continue;
            }
            auto paramMacroModeData = paramItem.modeParamMap.value(item.workMode);
            if (paramMacroModeData.pointFreqParamMap.isEmpty())
            {
                auto msg = QString("未查找到卫星代号为%1 工作模式为%2 的参数宏信息").arg(item.taskCode, workModeDesc);
                emit signalErrorMsg(msg);
                continue;
            }

            /* 跟踪 */
            if (item.workMode != XDS)
            {
                auto paramTrace = SystemOrientation::Unkonwn;
                auto sateTrace = SystemOrientation::Unkonwn;
                if (!MacroCommon::getTracePolarization(paramMacroModeData, item.pointFreqNo, paramTrace))
                {
                    auto msg =
                        QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的参数宏跟踪旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
                if (!sateItem.getSTrackPolar(item.workMode, item.pointFreqNo, sateTrace))
                {
                    auto msg =
                        QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的卫星跟踪旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
                if (paramTrace != sateTrace)
                {
                    auto msg = QString("任务代号为%1 工作模式为 %2 点频为 %3的参数宏跟踪旋向和卫星跟踪向不匹配")
                                   .arg(item.taskCode, workModeDesc)
                                   .arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
            }

            if (!(item.workMode == XDS || item.workMode == KaGS))
            {
                /* 发射 */
                auto paramSend = SystemOrientation::Unkonwn;
                auto sateSend = SystemOrientation::Unkonwn;
                if (!MacroCommon::getSendPolarization(paramMacroModeData, item.pointFreqNo, paramSend))
                {
                    auto msg =
                        QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的参数宏发射旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
                if (!sateItem.getLaunchPolar(item.workMode, item.pointFreqNo, sateSend))
                {
                    auto msg =
                        QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的卫星发射旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
                if (paramSend != sateSend)
                {
                    auto msg = QString("任务代号为%1 工作模式为 %2 点频为 %3的参数宏发射旋向和卫星发射旋向不匹配")
                                   .arg(item.taskCode, workModeDesc)
                                   .arg(item.pointFreqNo);
                    emit signalErrorMsg(msg);
                }
            }

            /* 接收旋向 */
            auto paramRecv = SystemOrientation::Unkonwn;
            auto sateRecv = SystemOrientation::Unkonwn;
            if (!MacroCommon::getRecvPolarization(paramMacroModeData, item.pointFreqNo, paramRecv))
            {
                auto msg =
                    QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的参数宏接收旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                emit signalErrorMsg(msg);
            }
            if (!sateItem.getSReceivPolar(item.workMode, item.pointFreqNo, sateRecv))
            {
                auto msg = QString("未查找任务代号为%1 工作模式为 %2 点频为 %3的卫星接收旋向").arg(item.taskCode, workModeDesc).arg(item.pointFreqNo);
                emit signalErrorMsg(msg);
            }
            if (paramRecv != sateRecv)
            {
                auto msg = QString("任务代号为%1 工作模式为 %2 点频为 %3的参数宏接收旋向和卫星接收旋向不匹配")
                               .arg(item.taskCode, workModeDesc)
                               .arg(item.pointFreqNo);
                emit signalErrorMsg(msg);
            }
        }
    }
}
