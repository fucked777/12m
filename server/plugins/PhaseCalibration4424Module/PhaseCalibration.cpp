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
#include "RedisHelper.h"
#include "SqlPhaseCalibration.h"
#include <QString>

class PhaseCalibrationImpl
{
public:
    cppmicroservices::BundleContext context;
    // SqlPhaseCalibration sql;
    static Optional<quint64> createDevNumber(SystemWorkMode workMode);
};

Optional<quint64> PhaseCalibrationImpl::createDevNumber(SystemWorkMode workMode)
{
    using ResType = Optional<quint64>;
    /* 生成设备组合号 */
    /* 1.获取当前配置宏数据 */
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        auto message = "查找当前模式对应的配置宏数据失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 2.获取当前测控基带 */
    DeviceID ckDeviceID;
    if (!MacroCommon::getMasterCKJDDeviceID(workMode, ckDeviceID))
    {
        auto message = "获取主用测控基带失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 3.获取当前跟踪基带 */
    DeviceID gzDeviceID;
    if (!MacroCommon::getGZQDDeviceID(workMode, gzDeviceID))
    {
        auto message = "获取主用跟踪基带失败";
        return ResType(ErrorCode::NotFound, message);
    }
    quint64 equipComb = 0;
    if (!EquipmentCombinationHelper::getCKDevNumber(workMode, configMacroData, ckDeviceID, gzDeviceID, equipComb))
    {
        auto message = "获取当前设备组合号失败";
        return ResType(ErrorCode::NotSupported, message);
    }
    return ResType(equipComb);
}

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
    registerSubscribe("AcceptDeviceCMDResultMessage", &PhaseCalibration::sg_deviceCMDResult, this);
}

PhaseCalibration::~PhaseCalibration() { delete m_impl; }

void PhaseCalibration::startPhaseCalibration(const ProtocolPack& pack)
{
    auto bak = pack;
    StartPhaseCalibration startPhaseCalibration;
    bak.data >> startPhaseCalibration;
    bak.data.clear();
    /* 20210621 wp?? 暂时不知道怎么搞 */
}
void PhaseCalibration::stopPhaseCalibration(const ProtocolPack& pack)
{ /* 20210621 wp?? 暂时不知道怎么搞 */
}
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
