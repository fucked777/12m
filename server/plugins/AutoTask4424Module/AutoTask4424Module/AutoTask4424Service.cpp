#include "AutoTask4424Service.h"

#include "CppMicroServicesUtility.h"

#include "MessagePublish.h"
#include "PlanRunMessageSerialize.h"
#include "SystemLogMessageSerialize.h"

#include "ControlFlowHandler.h"

#define MODULENAME "AutoTask4424Module"

AutoTask4424Service::AutoTask4424Service(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
{
    registerSubscribe("manualOperation", &AutoTask4424Service::manualOperation, this, false);
}

AutoTask4424Service::~AutoTask4424Service() {}

void AutoTask4424Service::manualOperation(const ProtocolPack& pack)
{
    auto bak = pack;
    ManualMessage msg;
    bak.data >> msg;

    UserID = bak.userID;

    ControlFlowHandler controlFlowHandler;
    connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &AutoTask4424Service::slotSendByteArrayToDevice);
    connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, this, &AutoTask4424Service::slotPublishInfoMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, this, &AutoTask4424Service::slotPublishWarningMsg);
    connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, this, &AutoTask4424Service::slotPublishErrorMsg);
    controlFlowHandler.handle(msg);
}

void AutoTask4424Service::slotSendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    this->silenceSend(protocal);
}

void AutoTask4424Service::slotPublishInfoMsg(const QString& msg) { SystemLogPublish::infoMsg(msg, UserID, MODULENAME); }

void AutoTask4424Service::slotPublishWarningMsg(const QString& msg) { SystemLogPublish::warningMsg(msg, UserID, MODULENAME); }

void AutoTask4424Service::slotPublishErrorMsg(const QString& msg) { SystemLogPublish::errorMsg(msg, UserID, MODULENAME); }
