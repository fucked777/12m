#include "AutoTask4426Service.h"

#include "ControlFlowHandler.h"
#include "CppMicroServicesUtility.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "PlanRunMessageDefine.h"
#include "PlanRunMessageSerialize.h"
#include "SystemLogMessageSerialize.h"
#include "TaskPlanMessageSerialize.h"
#include <QApplication>

AutoTask4426ServiceImpl::AutoTask4426ServiceImpl(QObject* parent)
    : QObject(parent)
{
    controlFlowHandler.setRunningFlag(&runningFlag);
}
AutoTask4426ServiceImpl::~AutoTask4426ServiceImpl() {}

void AutoTask4426ServiceImpl::manualOperation(const ManualMessage& msg)
{
    runningFlag = true;
    endFlag = false;

    manualMessage = msg;
    controlFlowHandler.handle(msg, true);

    runningFlag = false;
    endFlag = true;
    manualMessage = ManualMessage();
}

AutoTask4426Service::AutoTask4426Service(cppmicroservices::BundleContext /*context*/)
    : INetMsg(nullptr)
    , m_impl(new AutoTask4426ServiceImpl)
{
    qRegisterMetaType<ManualMessage>("ManualMessage");
    qRegisterMetaType<ManualMessage>("ManualMessage&");
    qRegisterMetaType<ManualMessage>("const ManualMessage&");

    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalSendToDevice, this, &AutoTask4426Service::slotSendByteArrayToDevice);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalInfoMsg, this, &AutoTask4426Service::slotPublishInfoMsg);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalWarningMsg, this, &AutoTask4426Service::slotPublishWarningMsg);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalErrorMsg, this, &AutoTask4426Service::slotPublishErrorMsg);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalSpecificTipsMsg, this, &AutoTask4426Service::slotPublishSpecificTipsMsg);

    connect(this, &AutoTask4426Service::sg_manualOperation, m_impl.data(), &AutoTask4426ServiceImpl::manualOperation);

    m_impl->moveToThread(&m_thread);
    m_thread.start();

    registerSubscribe("manualOperation", &AutoTask4426Service::manualOperation, this);
}

AutoTask4426Service::~AutoTask4426Service()
{
    m_impl->runningFlag = false;
    m_thread.quit();
    m_thread.wait();
}
void AutoTask4426Service::slotPublishInfoMsg(const QString& msg) { SystemLogPublish::infoMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME)); }
void AutoTask4426Service::slotPublishWarningMsg(const QString& msg) { SystemLogPublish::warningMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME)); }
void AutoTask4426Service::slotPublishErrorMsg(const QString& msg) { SystemLogPublish::errorMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME)); }
void AutoTask4426Service::slotPublishSpecificTipsMsg(const QString& msg)
{
    SystemLogPublish::specificTipsMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME));
}

void AutoTask4426Service::manualOperation(const ProtocolPack& pack)
{
    auto bak = pack;
    ManualMessage msg;
    bak.data >> msg;

    if (msg.manualType == ManualType::ResourceRelease)
    {
        if (msg.manualType == m_impl->manualMessage.manualType)
        {
            slotPublishWarningMsg("当前已有资源释放正在处理");
            return;
        }

        slotPublishInfoMsg("退出当前操作流程中...");
        m_impl->runningFlag = false;
        while (!m_impl->endFlag)
        {
            QThread::msleep(10);
            QApplication::processEvents();
        }
        slotPublishInfoMsg("退出操作流程完成");
    }
    if (m_impl->runningFlag)
    {
        slotPublishWarningMsg("当前有流程正在执行");
        return;
    }
    m_impl->userID = pack.userID;
    emit sg_manualOperation(msg);
}

void AutoTask4426Service::slotSendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    this->silenceSend(protocal);
}
