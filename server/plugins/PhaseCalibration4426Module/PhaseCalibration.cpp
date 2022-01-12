#include "PhaseCalibration.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroMessageDefine.h"
#include "CppMicroServicesUtility.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "PhaseCalibrationFlow.h"
#include "PhaseCalibrationSerialize.h"
#include "PhaseCalibrationService.h"
#include "RedisHelper.h"
#include "SqlPhaseCalibration.h"
#include <QApplication>
#include <QString>

class PhaseCalibrationImpl
{
public:
    cppmicroservices::BundleContext context;
    PhaseCalibrationService* phaseCalibrationService{ nullptr };
};

PhaseCalibration::PhaseCalibration(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new PhaseCalibrationImpl)
{
    m_impl->context = context;

    auto initDBRes = SqlPhaseCalibration::initDB();
    if (!initDBRes)
    {
        auto msg = QString("校零初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("startPhaseCalibration", &PhaseCalibration::startPhaseCalibration, this);
    registerSubscribe("stopPhaseCalibration", &PhaseCalibration::stopPhaseCalibration, this);
    registerSubscribe("deletePhaseCalibrationResult", &PhaseCalibration::deletePhaseCalibrationResult, this);
    registerSubscribe("clearHistry", &PhaseCalibration::clearHistry, this);
    registerSubscribe("queryHistry", &PhaseCalibration::queryHistry, this);
}

PhaseCalibration::~PhaseCalibration() { delete m_impl; }
void PhaseCalibration::startPhaseCalibration(const ProtocolPack& pack)
{
    if (m_impl->phaseCalibrationService != nullptr)
    {
        auto bak = pack;
        Optional<PhaseCalibrationItem> ack(ErrorCode::Busy, "当前已有校相正在进行");

        bak.data.clear();
        bak.data << ack;
        silenceSend(bak);
        return;
    }

    m_impl->phaseCalibrationService = new PhaseCalibrationService(this);
    connect(m_impl->phaseCalibrationService, &PhaseCalibrationService::sg_end, this, &PhaseCalibration::endPhaseCalibration);
    m_impl->phaseCalibrationService->startServer(pack);
}
void PhaseCalibration::endPhaseCalibration()
{
    if (m_impl->phaseCalibrationService != nullptr)
    {
        auto& service = m_impl->phaseCalibrationService;
        service->stopServer();
        while (service->isRunning() && !service->isFinished())
        {
            QApplication::processEvents();
            QThread::msleep(10);
        }
        service->wait();

        service->setParent(nullptr);
        delete service;
        service = nullptr;
    }
}
void PhaseCalibration::stopPhaseCalibration(const ProtocolPack& /*pack*/) { endPhaseCalibration(); }

void PhaseCalibration::deletePhaseCalibrationResult(const ProtocolPack& pack)
{
    auto bak = pack;
    PhaseCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = SqlPhaseCalibration::deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除%1校相结果成功").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除%1校相结果失败").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);
}

void PhaseCalibration::clearHistry(const ProtocolPack& pack)
{
    auto bak = pack;
    ClearPhaseCalibrationHistry item;
    bak.data >> item;
    bak.data.clear();
    auto ans = SqlPhaseCalibration::deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("清理90天内校相数据成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("清理90天内校相数据失败"), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);
}
void PhaseCalibration::queryHistry(const ProtocolPack& pack)
{
    auto bak = pack;
    QueryPhaseCalibrationHistry item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlPhaseCalibration::selectItem(item);
    silenceSendACK(bak);
}
